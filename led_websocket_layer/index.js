const WebSocket = require('ws');

const fs = require('fs');
const wss = new WebSocket.Server({
  port: 8080
});

wss.on('connection', ws => {
  ws.on('message', message => {
    console.log('received message ' + message);
    fs.writeFileSync(process.env.HOME + '/myPipe', message, err => {
      if (err) console.log(err);
    })
    ws.send('server wrote');
  });
  ws.send('connected');
});

