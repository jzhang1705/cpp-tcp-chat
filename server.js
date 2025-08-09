const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 5555 });

const clients = new Map(); // Map ws => username

wss.on('connection', (ws) => {
  let userName = '';

  ws.send('Welcome! Please send your name.');

  ws.on('message', (message) => {
    if (!userName) {
      userName = message.toString().trim();
      clients.set(ws, userName);
      broadcast(`${userName} joined the chat.`);
      return;
    }

    if (message.toString() === '/quit') {
      ws.close();
      return;
    }

    broadcast(`${userName}: ${message}`);
  });

  ws.on('close', () => {
    if (userName) {
      clients.delete(ws);
      broadcast(`${userName} left the chat.`);
    }
  });
});

function broadcast(msg) {
  for (let client of wss.clients) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(msg);
    }
  }
  console.log('Broadcast:', msg);
}

console.log('WebSocket server running on ws://localhost:5555');
