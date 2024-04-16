#include <WebSocketsClient.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

uint8_t newMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x66};

SoftwareSerial mySoftwareSerial(4, 5);
DFRobotDFPlayerMini myDFPlayer;

int audioFiles[13] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

int gpioPin = 12;
int gpioPin2 = 13;

const char* ssid = "SSIDhere";
const char* password = "Passwordhere";

WebSocketsClient webSocket;
bool isConnected = true;

int volumevalue = 0;

unsigned long previousMillis = 0;
int interval = 9000;

bool countdownActive = false;
bool currentlyplaying = false;

int filenumber = 1;
int audioFile = audioFiles[filenumber];

void hetisfeest(void) {
  myDFPlayer.play(audioFile);
  delay(200);
  digitalWrite(gpioPin, HIGH);
  previousMillis = millis(); // Reset the timer
  Serial.println("starttime is");
  Serial.println(previousMillis);
  countdownActive = true; // Start the countdown
  currentlyplaying = true;
}

void turnOff(void) {
  digitalWrite(gpioPin, LOW); // Turn off immediately
  myDFPlayer.stop();
  countdownActive = false; // Cancel the countdown
  currentlyplaying = false;
}

void piepje(void) {
  digitalWrite(gpioPin2, HIGH);
  delay(5);
  digitalWrite(gpioPin2, LOW);
}

void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void connectWebSocket() {
  Serial.println("Connecting to WebSocket...");
  webSocket.begin("Websockethere", port);
  webSocket.onEvent(onWebSocketEvent);
}

void reconnectWebSocket() {
  webSocket.disconnect();
  Serial.println("Reconnecting to WebSocket...");
  webSocket.begin("Websockethere", port);
  webSocket.onEvent(onWebSocketEvent);
}

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  
  Serial.print("[OLD] ESP8266 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  wifi_set_macaddr(STATION_IF, &newMACAddress[0]);
  
  Serial.print("[NEW] ESP8266 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  connectWiFi();
  connectWebSocket();

  WiFi.softAPdisconnect(true);
  Serial.println("Access point turned off");

  pinMode(gpioPin, OUTPUT);
  digitalWrite(gpioPin, LOW);

  pinMode(gpioPin2, OUTPUT);
  digitalWrite(gpioPin2, LOW);

  delay(1000);

  mySoftwareSerial.begin(9600);

  delay(1000);

  Serial.println(F("Initializing DFPlayer ..."));

  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1. Check the connection"));
    Serial.println(F("2. Insert the SD card"));
    while (true);
  }

  delay(500);

  Serial.println(F("DFPlayer Mini initialized."));
  myDFPlayer.volume(15);

  delay(500);
}

void onWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  if (type == WStype_DISCONNECTED) {
    Serial.println("DISCONNECTED! Connection to WebSocket: ");
    bool isConnected = false;
    Serial.print(isConnected);

  } else if (type == WStype_CONNECTED) {
    Serial.println("CONNECTED! Connection to WebSocket: ");
    bool isConnected = true;
    Serial.print(isConnected);
    piepje();
  } else if (type == WStype_ERROR) {
    Serial.println("ERROR! Connection to Websocket: ");
    bool isConnected = false;
    Serial.print(isConnected);

  } else if (type == WStype_PONG) {
    String message = String((char*)payload);
    Serial.println("Pong received");

  } else if (type == WStype_TEXT) {
    String message = String((char*)payload);
    Serial.println("Received message: ");
    Serial.print(message);

    if (message.startsWith("volume=")) {
      int value = message.substring(7).toInt(); // Extract the integer part after "volume="
      if (value >= 0 && value <= 30) {
        volumevalue = value; // Update the volumevalue with the received value
        Serial.print("Updated volumevalue to: ");
        Serial.println(volumevalue);
        myDFPlayer.volume(volumevalue);
      } else {
        Serial.println("Invalid volume value. Should be between 0 and 30.");
      }
    } else if (message == "on" && currentlyplaying == false) {
      Serial.println("DO ON");
      hetisfeest();
    } else if (message == "off") {
      Serial.println("DO OFF");
      turnOff();
    } else if (message == "stop") {
      Serial.println("DO STOP");
      turnOff();
    } else if (message.startsWith("audiofile=")) {
        int filevalue = message.substring(10).toInt(); // Extract the integer part after "volume="
        if (filevalue >= 1 && filevalue <= 9) {
          filenumber = filevalue; // Update the volumevalue with the received value
          audioFile = audioFiles[filenumber];
          Serial.print("Updated filenumber to: ");
          Serial.println(filenumber);
          interval = 10000;
          Serial.println(interval);
        } else if (filevalue >= 10 && filevalue <= 12) {
          filenumber = filevalue; // Update the volumevalue with the received value
          audioFile = audioFiles[filenumber];
          Serial.print("Updated filenumber to: ");
          Serial.println(filenumber);
          interval = 22000;
          Serial.println(interval);
        } else if (filevalue == 13) {
          filenumber = filevalue; // Update the volumevalue with the received value
          audioFile = audioFiles[filenumber];
          Serial.print("Updated filenumber to: ");
          Serial.println(filenumber);
          interval = 10000;
          Serial.println(interval);
        } else {
          Serial.println("Invalid file value. Should be between 0 and 13.");
        }
    }
  }
}

void checkTimer() {
  if (countdownActive == true) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      digitalWrite(gpioPin, LOW); // Turn off immediately
      countdownActive = false; // Cancel the countdown
      currentlyplaying = false;
    }
  }
}

void loop() {
  webSocket.loop();

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (countdownActive == true) {
    checkTimer();
  }

  if (isConnected == false) {
    reconnectWebSocket();
    delay(2000);
  }

  static unsigned long pingTimer = millis();
  if (millis() - pingTimer >= 30000) {
    webSocket.sendPing();
    Serial.println("Ping sent");
    pingTimer = millis();
  }

  if (!myDFPlayer.available()) {
    return;
  }

  uint8_t messageType = myDFPlayer.readType();
  uint16_t messageValue = myDFPlayer.read();

  checkTimer();

}