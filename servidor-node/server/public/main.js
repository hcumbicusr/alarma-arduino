const socket = io();
const password = "1234";

socket.on('estado_puerta', function (data) {
	// console.log(data);
	let estado = document.getElementById('estado_puerta');
	estado.innerHTML = data;
});
socket.on('estado_mampara', function (data) {
	// console.log(data);
	let estado = document.getElementById('estado_mampara');
	estado.innerHTML = data;
});
socket.on('estado_reja', function (data) {
	// console.log(data);
	let estado = document.getElementById('estado_reja');
	estado.innerHTML = data;
});
socket.on('alarma_encendida', function (data) {
	// console.log(data);
	let estado = document.getElementById('alarma_encendida');
	estado.innerHTML = data=='ON'? "SIRENA ENCENDIDA" : "SIRENA APAGADA";
	if (data == "ON") {
		document.getElementById("btn-panic-on").style.display = "none";
		document.getElementById("btn-panic-off").style.display = "block";
	} else {
		document.getElementById("btn-panic-on").style.display = "block";
		document.getElementById("btn-panic-off").style.display = "none";
	}
});
socket.on('alarma_activada', function (data) {
	// console.log(data);
	let estado = document.getElementById('alarma_activada');
	estado.innerHTML = data=='ON'? "ALARMA ACTIVADA" : "ALARMA DESACTIVADA";
	if (data == "ON") {
		document.getElementById("btn-security-on").style.display = "none";
		document.getElementById("btn-security-off").style.display = "block";
	} else {
		document.getElementById("btn-security-on").style.display = "block";
		document.getElementById("btn-security-off").style.display = "none";
	}
});

function sendSecurity(estado) {
	event.preventDefault();
	console.log(estado);
	let data = {
		'PASSWORD': password,
		'MODE': estado
	};

	let request = new XMLHttpRequest();
	request.open('POST', '/security', true);
	// request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8');
	request.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');

	request.onreadystatechange = function() {
	  if (this.readyState == XMLHttpRequest.DONE && this.status == 200) {
	    console.log('succeed', request.responseText);
	    // myresponse.value = request.responseText;
	  } else {
	    console.log('server error');
	  }
	};

	request.onerror = function() {
	  console.log('something went wrong');
	};

	request.send(JSON.stringify(data));
}

function sendPanic(estado) {
	event.preventDefault();
	console.log(estado);
	let data = {
		'PASSWORD': password,
		'MODE': estado
	};

	let request = new XMLHttpRequest();
	request.open('POST', '/panic', true);
	// request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded; charset=UTF-8');
	request.setRequestHeader('Content-Type', 'application/json; charset=UTF-8');

	request.onreadystatechange = function() {
	  if (this.readyState == XMLHttpRequest.DONE && this.status == 200) {
	    console.log('succeed', request.responseText);
	    // myresponse.value = request.responseText;
	  } else {
	    console.log('server error');
	  }
	};

	request.onerror = function() {
	  console.log('something went wrong');
	};

	request.send(JSON.stringify(data));
}

