import websocket
import netifaces
import requests
import sys
import json

def getIP():
	# Obtiene la lista de interfaces de red
	interfaces = netifaces.interfaces()

	for interface in interfaces:
		# Obtiene la dirección IP de la interfaz
		address = netifaces.ifaddresses(interface)

		if interface.startswith("wl"):
			# Verifica si hay direcciones IPv4
			if netifaces.AF_INET in address:
				IPinfo = address[netifaces.AF_INET][0]
				ip = IPinfo['addr']
				return ip

	return "0.0.0.0"

def modIP(ip, modIP):
	split = ip.split(".") # Divide la dirección IP en partes
	split[-1] = str(modIP)  # Cambia el último dígito

	newIP = ".".join(split) # Une las partes de nuevo en una dirección IP
	return newIP

def on_open(WS):
	print("Conectado al servidor WebSocket")

	response = requests.get('http://' + arduinoIP + '/config.json') # Realizar la solicitud GET con la URL del archivo JSON en el servidor

	# Verificar que la solicitud fue exitosa
	if response.status_code == 200:
		data = response.json() # Cargar los datos del archivo JSON
		PowerStatus = data['PowerStatus']

		# Modificar los datos del archivo JSON
		try:
			PowerStatus['power'] = int(sys.argv[1])
			PowerStatus['connected'] = int(sys.argv[1])
		except ValueError:
			PowerStatus['power'] = 0
			PowerStatus['connected'] = 0
		WS.send(json.dumps(data)) # Enviar los nuevos datos JSON al WebSocket
	else:
		print(f'Error al acceder al archivo: {response.status_code}')

def on_message(WS, message):
	print(f"Dato recibido: {message}")

def on_error(WS, error):
	print(f"Error: {error}")

def on_close(WS, status_code, reason):
	print("Desconectado del servidor WebSocket")

arduinoIP = modIP(getIP(), "10")

if __name__ == "__main__":
	#print(f"La dirección IP del arduino es: {arduinoIP}")

	WS = websocket.WebSocketApp(	"ws://" + arduinoIP + ":81",
					on_open = on_open,
					on_message = on_message,
					on_error = on_error,
					on_close = on_close)
	WS.run_forever()
