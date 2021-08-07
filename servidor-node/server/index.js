const http = require('http');
// const https = require('https');
const express = require('express');
const socketIO = require('socket.io');
const bodyParser = require("body-parser");
const router = express.Router();

const http_port = 3000;
const app = express();
const server = http.createServer(app);
const io = socketIO(server);
const ARDUINO_HOST = '192.168.1.160';
const ARDUINO_PORT = 80;
const COM = 'COM3'; // En Windows= COM3, en linux debe indicar la ruta

app.use(express.static(__dirname + '/public'));

server.listen(http_port, function () {
	console.log('server listening on port', http_port)
});

// SERIAL COMUNICATION: para consultar la salida serial de arduino

const Serialport = require('serialport');
const Readline = Serialport.parsers.Readline;
const port = new Serialport(COM, {
	baudRate: 9600
});
const parser = port.pipe(new Readline({ delimeter: '\r\n' }));
parser.on('open', function () {
	console.log("Conexión abierta");
});
parser.on('data', function (data) {
	emitirData(data);
});
port.on('error', function (err) {
	console.log(err);
});


function emitirData(data) {
	console.log(data);
	let estados = [];
	let flags = {
		'alarma_encendida': 'OFF',
		'alarma_activada': 'OFF',
	};
	try {
		estados = JSON.parse("["+data.substring(0, data.lastIndexOf("}")+1)+"]");
	} catch (e) {
		console.error(e)
	}
	try {
		flags = JSON.parse("{"+data.substring(data.lastIndexOf("},")+2)+"}");
	} catch (e) {
		console.error(e)
	}
	
	// let flags = data.substring(data.lastIndexOf("},")+2);
	// console.log(flags)
	for(i in estados) {
		if ( estados[i].sensor == 'PUERTA')
			io.emit('estado_puerta', estados[i].estado);
		if ( estados[i].sensor == 'MAMPARA')
			io.emit('estado_mampara', estados[i].estado);
		if ( estados[i].sensor == 'REJA')
			io.emit('estado_reja', estados[i].estado);
	}
	io.emit('alarma_encendida', flags.alarma_encendida);
	io.emit('alarma_activada', flags.alarma_activada);
}

// ---------------- HTTP ----------------
// Parse JSON bodies (as sent by API clients)
app.use(express.json());

function checkoutArduino() {
	const data = {
		'ACTION': 'SERIAL',
	};
	const url_params = objectToUrlParams(data);
	const options = {
	  hostname: ARDUINO_HOST,
	  port: ARDUINO_PORT,
	  path: '/?'+url_params,
	  method: 'GET'
	}
	// console.log(options);

	const req = http.request(options, res => {
	  // console.log(`statusCode: ${res.statusCode}`);
	  // console.log(res);

	  res.on('data', d => {
	    // process.stdout.write(d);
		emitirData(""+d);
	  })
	})

	req.on('error', error => {
	  console.error(error);
	})

	req.end();
}
// setInterval(checkoutArduino, 1000); // para consultar por HTTP

app.post('/security', function (req, res) {
	const data = {
		'ACTION': 'SECURITY',
		'MODE': req.body.MODE,
		'PASSWORD': req.body.PASSWORD,
	};
	console.log(data);
	const params = objectToUrlParams(data);
	sendArduino(params);
	res.end("OK");
});

app.post('/panic', function (req, res) {
	const data = {
		'ACTION': 'PANIC',
		'MODE': req.body.MODE,
		'PASSWORD': req.body.PASSWORD,
	};
	console.log(data);
	const params = objectToUrlParams(data);
	sendArduino(params);
	res.end("OK");
});

function sendArduino(url_params) {
	const options = {
	  hostname: ARDUINO_HOST,
	  port: ARDUINO_PORT,
	  path: '/?'+url_params,
	  method: 'GET'
	}
	// console.log(options);

	const req = http.request(options, res => {
	  console.log(`statusCode: ${res.statusCode}`);

	  res.on('data', d => {
	    process.stdout.write(d);
	  })
	})

	req.on('error', error => {
	  console.error(error);
	})

	req.end();
}

function objectToUrlParams(obj) {
	var str = "";
	for (var key in obj) {
	    if (str != "") {
	        str += "&";
	    }
	    str += key + "=" + encodeURIComponent(obj[key]);
	}
	return str;
}