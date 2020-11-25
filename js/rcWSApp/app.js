#!/usr/bin/env node

const  wsUri = "ws://10.2.113.123/ws";
let WebSocketClient = require('websocket').client;

function heartbeat(connection)
{
  clearTimeout(this.pingTimeout);

  this.pingTimeout = setTimeout(() => {
	console.log('WebSocket server on '+wsUri+ ' is not responding!');
    connection.drop();
	console.log('Try to reconnect...');
	client.connect(wsUri);
  }, 3000 + 1000);
}

function wsConnect(connection)
{
    console.log('WebSocket Client Connected to '+wsUri);
	heartbeat(connection);
    connection.on('error', function(error) {
        console.log("Connection Error: " + error.toString());
    });
    connection.on('close', function() {
        console.log('Connection Closed');
    });
    connection.on('message', function(message)
	{
		let rcCode;
		const now = new Date();
		const options =
		{ 
			year: 'numeric',
			month: '2-digit',
			day: '2-digit',
			hour: '2-digit',
			minute: '2-digit',
			second: '2-digit'
		};
	
		if (message.type == 'binary')
		{		
			rcCode = message.binaryData.readUInt32LE(0);
			console.log('binary '+now.toLocaleDateString("ru-RU",options) + ' --> ' + rcCode);
		}

		if (message.type == 'utf8')
		{
			rcCode = message.utf8Data;
			console.log('text '+now.toLocaleDateString("ru-RU",options) + ' --> ' + rcCode);
		}
    });
	
	connection.on('pong', function(message)
	{
		console.log('Got pong from server!');
	});
	connection.on('ping', function(message)
	{
		console.log('Got ping from server!');
		heartbeat(connection);
	});
}
 
let client = new WebSocketClient();
 
client.on('connectFailed', function(error) {
    console.log('Connect Error: ' + error.toString());
});
 
client.on('connect', wsConnect);
 
client.connect(wsUri);