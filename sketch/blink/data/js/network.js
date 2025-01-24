var data;
var WiFi_enable, AP_enable;
var WiFi_credentials, WiFi_DHCP, WiFi_addresses;
var AP_credentials, AP_DHCP, AP_addresses;

// Aquí puedes cargar el archivo JSON o enviar un mensaje inicial si es necesario
function fetchJConfig() {
	fetch("../config/config.json")
		.then(response => response.json())
		.then(content => {
			data = content;
			
			WiFi_enable = data.WiFi.enable;
			WiFi_credentials = data.WiFi.credentials;
			WiFi_addresses = data.WiFi.addresses;
			WiFi_DHCP = data.WiFi.dhcp;
			AP_enable = data.AP.enable;
			AP_credentials = data.AP.credentials;
			AP_addresses = data.AP.addresses;
			AP_DHCP = data.AP.dhcp;
			
			initToggle("wifi");
			initToggle("ap");
			
			getJdata("wifi", true);
			getJdata("ap", true);
		})
		.catch(error => console.error("Error al cargar el JSON:", error));
}

// Llamar a la función al cargar la página para establecer el estado inicial
window.onload = function() {
	getFields("wifi");
	getFields("ap");
	initErrorField();
};

var wifi = new Array(), ap = new Array();
function getFields(value){
	var IDs = [
		value + "-enable",
		value + "-dhcp",
		value + "-ssid",
		value + "-pass",
		value + "-address",
		value + "-gateway",
		value + "-subnet"
	];
	
	switch(value){
		case "wifi": for (let dns = 1; dns <=2; dns++) IDs.push(value + "-dns" + dns); break;
		case "ap": for (let type = 1; type <=2; type++) IDs.push(value + (type === 1 ? "-since" : "-dns")); break;
	}
	
	for (let ID of IDs) (value === "wifi" ? wifi : ap).push(document.getElementById(ID));
}

function initErrorField(){ for (let fields of [wifi, ap]) for (let field of fields) errorField(field);}
function errorField(field) {
	field.addEventListener("input", function(event) {
		const efield = event.target;
		
		if ((isValid(field) === null)) addErrorField(efield,`${efield.name} no válida`);
		else removeErrorField(efield);
	});
}
function addErrorField(field, message) {
	field.classList.add("field_error");
	field.nextElementSibling.classList.add("error");
	field.nextElementSibling.innerText = message;
}
function removeErrorField(field) {
	field.classList.remove("field_error");
	field.nextElementSibling.classList.remove("error");
	field.nextElementSibling.innerText = "";
}

// Función para validar un SSID
function isValidSSID(ssid) {
	ssid = ssid.trim();
	return ssid.length !== 0 && ssid.length <= 32 ? ssid : null; // Verifica si la longitud del ssid es 32 o menos caracteres
}
// Función para validar una contraseña
function isValidPASS(pass) {
	return pass.length >= 8 ? pass : null; // Verifica si la longitud de la contraseña es 8 o más caracteres
}
// Función para validar una dirección IP
function isValidIP(ip) {
	const ipPattern = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
	return ipPattern.test(ip) ? ip : null; // Retorna la IP si es válida, de lo contrario retorna null
}
function isValid(field){
	var value;
	switch (field.getAttribute("name")) {
		case "SSID": value = isValidSSID(field.value); break;
		case "Contraseña": value = isValidPASS(field.value); break;
		case "DHCP":
		case "Habilitado":
			break;
		case "DNS":
			if (field.value.trim() === "") {
				value = field.value.trim();
				break;
			}
		default: value = isValidIP(field.value); break;
	}
	return value;
}

function fetchArrObj(value) {
	let attemp = 0;
	if (Socket.readyState === WebSocket.OPEN) {
		if ((WiFi_credentials || AP_credentials) && (WiFi_credentials.ssid || AP_credentials.ssid)) {
			var ArrObj = {};
			const field = (value === "wifi" ? wifi : ap);
			
			// Objeto con toda la información a ser establecida y a establecer
			switch(value) {
				case "wifi":
					ArrObj.array = [
						WiFi_enable,
						WiFi_DHCP,
						WiFi_credentials.ssid,
						WiFi_credentials.pass,
						WiFi_addresses.address,
						WiFi_addresses.gateway,
						WiFi_addresses.subnet,
						WiFi_addresses.dns1,
						WiFi_addresses.dns2
					];
					break;
				case "ap":
					ArrObj.array = [
						AP_enable,
						AP_DHCP,
						AP_credentials.ssid,
						AP_credentials.pass,
						AP_addresses.address,
						AP_addresses.gateway,
						AP_addresses.subnet,
						AP_addresses.since,
						AP_addresses.dns
					];
					break;
			}
			
			ArrObj.object = {
				enable: field[enablePOS].checked,
				dhcp: field[dhcpPOS].checked,
				credentials: {
					ssid: field[2].value,
					pass: field[3].value
				},
				addresses: {
					address: field[4].value,
					gateway: field[5].value,
					subnet: field[6].value
				}
			};
			
			switch(value){
				case "wifi":
					ArrObj.object.addresses.dns1 = field[7].value;
					ArrObj.object.addresses.dns2 = field[8].value;
					break;
				case "ap":
					ArrObj.object.addresses.since = field[7].value;
					ArrObj.object.addresses.dns = field[8].value;
					break;
			}
			
			return ArrObj; // Retornar el objeto con array y objeto
		}
		else return { array: [], object: {} }; // Retornar un objeto vacío
	}
	else {
		if (attemp++ === 10) return { array: [], object: {} }; // Retornar un objeto vacío si no está conectado después de 10 intentos
		
		setTimeout(() => this, 1000); // Intentar reconectar después de 1 segundo
		return { array: [], object: {} }; // Retornar un objeto vacío si no está
	}
}

function getJdata(value, all) {
	const { array: JValue } = fetchArrObj(value); // Desestructurar el array

	if (JValue.length !== 0) {
		const field = (value === "wifi" ? wifi : ap);
		for (let i = (!all ? dhcpPOS + 3 : dhcpPOS + 1); i < field.length; i++) field[i].value = JValue[i];
	}
}

function setJdata(value) {
	const { object: JValue } = fetchArrObj(value); // Desestructurar el objeto
	
	if (Object.keys(JValue).length !== 0) {
		switch(value) {
			case "wifi": data.WiFi = JValue; break;
			case "ap": data.AP = JValue; break;
		}
	}
}

(function() {
	// Función para limpiar el formulario
	document.getElementById("wifi-reset").addEventListener("click", resetForm.bind(null, "wifi"));
	document.getElementById("ap-reset").addEventListener("click", resetForm.bind(null, "ap"));
	
	function resetForm(value){
		document.getElementById(value + "-form").reset(); // Restablece todos los campos del formulario
	}
})();

// Función para enviar el formulario
(function() {
	document.getElementById("wifi-submit").addEventListener("click", () => {upload("wifi");});
	document.getElementById("ap-submit").addEventListener("click", () => {upload("ap");});
	
	function upload(value){
		const fields = (value === "wifi" ? wifi : ap);
		let correct = true;
		
		for (let field of fields) if ((field.classList.contains("field_error")) && (field.nextElementSibling.classList.contains("error"))) correct = false;
		
		if (correct) {
			// Verifica si el WebSocket está abierto antes de enviar
			if (Socket.readyState === WebSocket.OPEN) {
				setJdata(value);
				
				// console.log(data);
				
				Socket.send(JSON.stringify(data)); // Envia los valores en data al .json
				progressBar();
			}
			else console.error("WebSocket no está abierto. Estado:", Socket.readyState);
		}
	}
})();

// Función para mostrar u ocultar campos según la selección de DHCP
function toggleENABLE(value) {
	const field = (value === "wifi" ? wifi : ap);
	
	for (let i = enablePOS + 1; i < field.length; i++) field[i].disabled = !field[enablePOS].checked;
}
// Evento para cambiar la visibilidad al cambiar el selector de Habilitado
(function() {
	document.getElementById("wifi-enable").addEventListener("change", toggleENABLE.bind(null, "wifi"));
	document.getElementById("ap-enable").addEventListener("change", toggleENABLE.bind(null, "ap"));
})();


var enablePOS, dhcpPOS;
function initToggle(value) {
	const field = (value === "wifi" ? wifi : ap);
	
	for (let i = 0; i < field.length; i++) {
		switch (field[i].getAttribute("name")) {
			case "Habilitado": enablePOS = i; break;
			case "DHCP": dhcpPOS = i; break;
		}
	}
	
	field[enablePOS].checked = (value === "wifi" ? WiFi_enable : AP_enable);
	field[dhcpPOS].checked = (value === "wifi" ? WiFi_DHCP : AP_DHCP);
	toggleDHCP(value);
	toggleENABLE(value);
}
// Función para mostrar u ocultar campos según la selección de DHCP
function toggleDHCP(value) {
	const field = (value === "wifi" ? wifi : ap);

	const isDHCP = field[dhcpPOS].checked;
	const display = (isDHCP ? "none" : "block");
	
	if (!isDHCP) getJdata(value, false);
	else for(let i = dhcpPOS + 3; i < field.length; i++) field[i].value = "";
	
	var fieldGroup = [
		value + "-since-group",
		value + "-address-group",
		value + "-gateway-group",
		value + "-subnet-group"
	];
	
	switch(value){
		case "wifi": for (let dns = 1; dns <=2; dns++) fieldGroup.push(value + "-dns" + dns + "-group"); break;
		case "ap": fieldGroup.push(value + "-dns-group"); break;
	}
	
	for (let i = (value === "ap" ? 0 : 1); i < fieldGroup.length; i++) document.getElementById(fieldGroup[i]).style.display = display;
}
// Evento para cambiar la visibilidad al cambiar el selector de DHCP
(function() {
	document.getElementById("wifi-dhcp").addEventListener("change", toggleDHCP.bind(null, "wifi"));
	document.getElementById("ap-dhcp").addEventListener("change", toggleDHCP.bind(null, "ap"));
})();
