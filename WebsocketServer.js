const http = require('http');
const WebSocket = require('ws');

let powerState = 'off';
let volume = 0;
let audiofile = 1;
let lastPongTime = null;

const server = http.createServer((req, res) => {
  if (req.method === 'GET') {
    const url = new URL(req.url, `http://${req.headers.host}`);

    switch (url.pathname) {
      case '/on':
        powerState = 'on';
        break;
      case '/off':
        powerState = 'off';
        break;
      case '/stop':
        powerState = 'stop';
        break;
      case '/volume':
        const newVolume = parseInt(url.searchParams.get('value'), 10);
        if (!isNaN(newVolume) && newVolume >= 0 && newVolume <= 30) {
          volume = newVolume;
        }
        break;
	  case '/audiofile':
        const newAudiofile = parseInt(url.searchParams.get('value'), 10);
        if (!isNaN(newAudiofile) && newAudiofile >= 1 && newAudiofile <= 13) {
          audiofile = newAudiofile;
        }
        break;
    }

    res.writeHead(200, { 'Content-Type': 'text/plain' });
    if (url.pathname === '/volume') {
      res.end(`volume=${volume}`);
	} else if (url.pathname === '/audiofile') {
      res.end(`audiofile=${audiofile}`);
    } else {
      res.end(powerState);
    }
  } else {
    res.writeHead(405, { 'Allow': 'GET' });
    res.end();
  }
});

server.listen(port, 'localhost', () => {
  console.log(`Web server running at http://localhost:porthere`);
});

const wss = new WebSocket.Server({ port: porthere });

wss.on('connection', (ws) => {
  // Send "new connection" to the client upon connection
  ws.send('new connection');

  ws.on('ping', () => {
    ws.pong();
    lastPongTime = Date.now();
  });
});

server.on('request', (req) => {
  if (req.method === 'GET') {
    const url = new URL(req.url, `http://${req.headers.host}`);

    switch (url.pathname) {
      case '/on':
      case '/off':
      case '/stop':
        wss.clients.forEach((client) => {
          if (client.readyState === WebSocket.OPEN) {
            client.send(powerState);
          }
        });
        break;
      case '/volume':
        wss.clients.forEach((client) => {
          if (client.readyState === WebSocket.OPEN) {
            client.send(`volume=${volume}`);
          }
        });
        break;
      case '/audiofile':
        wss.clients.forEach((client) => {
          if (client.readyState === WebSocket.OPEN) {
            client.send(`audiofile=${audiofile}`);
          }
        });
        break;
      default:
        return;
    }
  }
});

setInterval(() => {
  if (lastPongTime) {
    const elapsedSeconds = Math.round((Date.now() - lastPongTime) / 1000);
    process.stdout.write(`Last ping received ${elapsedSeconds} seconds ago   \r`);
  }
}, 1000);
