#include <Arduino.h>

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>  // needed for instant communication between client and server through Websockets

#include <ArduinoJson.h>       // needed for JSON encapsulation (send multiple variables with one string)

#include <FS.h>
#include <LittleFS.h>

#include <vector>
#include <cstring>
#include <map>

#include "utils.h"
#include "functions.h"
#include "init.h"

// Setup everything to make the webserver work.
void setup(void) {
	pinMode(Contact1, OUTPUT);  // Configura el pin como salida
	pinMode(Contact2, INPUT);   // Configura el pin como entrada
	digitalWrite(Contact2, HIGH);

	delay(3000);  // wait for serial monitor to start completely.

	// Use Serial port for some trace information
	Serial.begin(115200);
	Serial.setDebugOutput(false);

	TRACE("\n\n");

	initFS();

	initWiFi();

	initServer();

}  // setup

// run the server...
void loop(void) {
	server.handleClient();
	webSocket.loop();  // Update function for the webSockets
	initVeriPower();
}  // loop()
