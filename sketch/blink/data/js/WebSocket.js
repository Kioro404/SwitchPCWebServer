var Socket; // Declaración de la variable que será el WebSocket

Socket = new WebSocket('ws://' + window.location.hostname + ':81/');

function initWebSocket() {
	Socket.onopen = function(event) {
		console.log("WebSocket abierto:", event);
		fetchJConfig();
	};
	
	Socket.onclose = function(event) {
		console.log("WebSocket cerrado:", event);
		setTimeout(() => initWebSocket(), 1000); // Intentar reconectar después de 1 segundos si hay un retraso
	};
	
	Socket.onerror = function(error) {
		console.error("WebSocket error:", error);
		Socket.close(); // Cerrar el socket
	};
	
	// Socket.onmessage = function(event) {
	// 	var Jvalue = JSON.parse(event.data);
	// 	// document.getElementById('rand1').innerHTML = obj.rand1;
	// 	isPower = Jvalue.isPower;
	// 	console.log(isPower);
	// 	upload(isPower, "Dispositivo encendido");
	// };
}

initWebSocket();