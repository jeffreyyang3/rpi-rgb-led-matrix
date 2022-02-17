const WebSocket = require('ws');
const redis = require('redis');
const fs = require('fs');
const wss = new WebSocket.Server({
  port: 8080
});
const publisher = redis.createClient();
publisher.connect();

wss.on('connection', ws => {
  ws.on('message', message => {
    console.log('received message ' + message);
    publisher.publish('dataChannel', message);
    ws.send('server wrote');
  });
  ws.send('connected');
});

