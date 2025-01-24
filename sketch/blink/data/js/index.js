var PowerStatus, data;

// Aquí puedes cargar el archivo JSON o enviar un mensaje inicial si es necesario
function fetchJConfig() {
	fetch("../config/config.json")
		.then(response => response.json())
		.then(content => {
			data = content;
			PowerStatus = data.PowerStatus;
			isPower = PowerStatus.power;
			replace(isPower);
		})
		.catch(error => console.error('Error al cargar el JSON:', error));
}

function replace(power){
	switch (power) {
		case 0:
			button.classList.remove("bg-on");
			button.classList.add("bg-off");
			break;
		case 1:
			button.classList.remove("bg-off");
			button.classList.add("bg-on");
			break;
		case 2:
			button.classList.remove("bg-on");
			button.classList.remove("bg-off");
			button.classList.add("bg-off");
			break;
	}
}

function upload(power, message) {
	// Ejecutar la acción basada en el valor de isPower
	console.log(message);
	
	replace(power);
	
	// Verifica si el WebSocket está abierto antes de enviar
	if (Socket.readyState === WebSocket.OPEN) {
		PowerStatus.power = power
		PowerStatus.message = message
		Socket.send(JSON.stringify(data)); // Envia los valores en data al .json
	}
	else console.error("WebSocket no está abierto. Estado:", Socket.readyState);
}

var isPower;
const button = document.getElementById("Power_on-off");

document.addEventListener("DOMContentLoaded", function() {
	let timer, longPress = false;
	
	// Evento para actualizar el contenido del botón
	button.addEventListener("pointerup", function() {
		if (!longPress) {
			isPower = isPower === 1 ? 0 : 1; // Alternar entre 0 y 1
			
			upload(isPower, "Dispositivo " + (isPower === 1 ? "encendido" : "apagado"));
		}
	}); // "pointerup" como sustituto de "click" para que funcione sin problemas en touchpads
	
	// Función para el apagado forzado
	function ForcePowerOFF() {

		longPress = false;
		timer = setTimeout(function () {
			longPress = true;
			isPower = 2;
			
			upload(isPower, "Dispositivo apagado forzado");
		}, 1000); // 1000 milisegundos = 1 segundos
	}
	
	button.addEventListener("mousedown", ForcePowerOFF); // Manejo de eventos para mouse
	button.addEventListener("touchstart", function(event) {
		event.preventDefault(); // Previene el comportamiento predeterminado
		ForcePowerOFF();
	}); // Manejo de eventos para pantallas táctiles
	
	// Función para limpiar el temporizador
	function clearTimer() {
		clearTimeout(timer);
	}
	
	// clearTimeout
	button.addEventListener("mouseup", clearTimer);
	button.addEventListener("mouseleave", clearTimer);
	button.addEventListener("touchend", clearTimer);
	button.addEventListener("touchcancel", clearTimer);
});
