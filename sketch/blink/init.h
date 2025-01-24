JsonDocument Jdoc; // create a JSON container

void initFS(){
	if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) TRACE("Montado de LittleFS fallido\n"); // Formatea los archivos del sistema en caso de no estarlo
	else {
		const std::vector<const char*> path = {
			"/assets/icons",
			"/config",
			"/css",
			"/js",
			"/pages"
		};
		TRACE("Listando directorios\n");
		for (int i = 0; i < path.size(); i++) listDir(path[i], 0); // Muestra los archivos en la memoria flash
	}
}

int cont = 0;
bool connectedPC = false;
bool veri = true;

void initVeriPower() {
	JsonObject PowerStatus = Jdoc["PowerStatus"];
	if (((PowerStatus["power"] == 1) && (PowerStatus["connected"] == 0) && (!connectedPC))) {
		// TRACE("%s%u\n", (cont < 10) ? "0" : "", cont);
		delay(1000);
		if (cont++ == 70) {
			cont = 0;
			veri = true;
			pressPower(P_FORCE);
			delay(3000);
			pressPower(P_NORMAL);
			TRACE("Reinicio forzado\n");
		}
	}
	else if ((PowerStatus["power"] == 1) && (PowerStatus["connected"] == 1)) {
		if (connectedPC) {
			if (veri) {
				cont = 0;
				veri = false;
				TRACE("Conexión establecida correctamente\n");
			}
		}
		else {
			if (veri) {
				veri = false;

				modJFile("connected", "0");
				modJFile("power", "0");

				TRACE("Hubo un problema, restableciendo los valores de inicio\n");
			}
		}
	}
}

std::vector<std::vector<String>> JdocWiFiOrg, JdocAPOrg;
	
void initWiFi() {
	// static int attemp = 0;
	// Network.onEvent([] (arduino_event_id_t event, arduino_event_info_t info) {
	//   TRACE(" ");
	//   switch (event) {
	//     case ARDUINO_EVENT_WIFI_STA_START:          TRACE("Wi-Fi iniciado"); break;
	//     case ARDUINO_EVENT_WIFI_STA_CONNECTED:      TRACE("Wi-Fi conectado"); break;
	//     case ARDUINO_EVENT_WIFI_STA_GOT_IP:         // TRACE("Wi-Fi IP asignada: %s", WiFi.localIP().toString().c_str());
	//       // WiFi.AP.enableNAPT(true);
	//       break;
	//     case ARDUINO_EVENT_WIFI_STA_LOST_IP:        TRACE("Wi-Fi perdió la dirección IP");
	//       // WiFi.AP.enableNAPT(false);
	//       break;
	//     case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:   if (attemp++ == 0) { TRACE("Wi-Fi desconectado");
	//       // WiFi.AP.enableNAPT(false);
	//       }
	//       break;
	//     case ARDUINO_EVENT_WIFI_STA_STOP:           TRACE("Wi-Fi detenido"); break;

	//     case ARDUINO_EVENT_WIFI_AP_START:           TRACE("AP iniciado"); break;
	//     case ARDUINO_EVENT_WIFI_AP_STACONNECTED:    TRACE("AP conectado"); break;
	//     case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED: TRACE("AP desconectado"); break;
	//     case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:   TRACE("AP IP asignada: %s", IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str()); break;
	//     case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:  TRACE("AP recibiendo respuesta"); break;
	//     case ARDUINO_EVENT_WIFI_AP_STOP:            TRACE("AP detenido"); break;

	//     default: break;
	//   }
	//   TRACE("\n");
	// });
	
	DeserializationError error = deserializeJson(Jdoc, readFile("/config/config.json"));
	if (error) {
		TRACE(F("deserializeJson() fallido: %s"), error.f_str());
		return;
	}
	else {
		JsonObject WiFi_addresses = Jdoc["WiFi"]["addresses"], WiFi_credentials = Jdoc["WiFi"]["credentials"];
		JsonObject AP_addresses = Jdoc["AP"]["addresses"], AP_credentials = Jdoc["AP"]["credentials"];

		JdocWiFiOrg = {   // Original
			{ // enable
				Jdoc["WiFi"]["enable"]
			},
			{ // dhcp
				Jdoc["WiFi"]["dhcp"]
			},
			{ // credentials
				WiFi_credentials["ssid"],
				WiFi_credentials["pass"]
			},
			{ // addresses
				WiFi_addresses["address"],
				WiFi_addresses["gateway"],
				WiFi_addresses["subnet"],
				WiFi_addresses["dns1"],
				WiFi_addresses["dns2"]
			}
		};
		JdocAPOrg = {
			{ // enable
				Jdoc["AP"]["enable"]
			},
			{ // dhcp
				Jdoc["AP"]["dhcp"]
			},
			{ // credentials
				AP_credentials["ssid"],
				AP_credentials["pass"]
			},
			{ // addresses
				AP_addresses["address"],
				AP_addresses["gateway"],
				AP_addresses["subnet"],
				AP_addresses["since"],
				AP_addresses["dns"]
			}
		};

		// TRACE("%s\n", readFile(dirFile[Jconfig]));

		if ((Jdoc["WiFi"]["enable"] == true) || (Jdoc["AP"]["enable"] == true)) {
			const char* HOSTNAME = Jdoc["Hostname"]; // name of the server.
			TRACE("\nIniciando %s\n\n", HOSTNAME);
			
			if ((Jdoc["WiFi"]["enable"] == true) && (Jdoc["AP"]["enable"] == false)) WiFi.mode(WIFI_STA);
			else if ((Jdoc["WiFi"]["enable"] == false) && (Jdoc["AP"]["enable"] == true)) WiFi.mode(WIFI_AP);
			else if ((Jdoc["WiFi"]["enable"] == true) && (Jdoc["AP"]["enable"] == true)) WiFi.mode(WIFI_AP_STA);

			if (Jdoc["WiFi"]["enable"] == true) {
				if (Jdoc["WiFi"]["dhcp"] == false) {
					WiFi.config(toIPAddress(WiFi_addresses["address"]), toIPAddress(WiFi_addresses["gateway"]), toIPAddress(WiFi_addresses["subnet"]));
					
					if ((strcmp(WiFi_addresses["dns1"], "") != 0) && (strcmp(WiFi_addresses["dns2"], "") != 0)) WiFi.setDNS(toIPAddress(WiFi_addresses["dns1"]), toIPAddress(WiFi_addresses["dns2"]));
				}
				
				WiFi.setHostname(HOSTNAME);
				WiFi.begin((const char*)WiFi_credentials["ssid"], (const char*)WiFi_credentials["pass"]); // Iniciar la conexión del WiFi

				int attemp = 0;
				while ((WiFi.status() != WL_CONNECTED) && (attemp++ <= 10)) delay(500); // Espera a que se conecte el Wi-Fi
				if (WiFi.status() != WL_CONNECTED) WiFi.disconnect(); // Desconecta el Wi-Fi en caso de no haberse establecido la conexión
				else {
					WiFi.setAutoReconnect(true);
					WiFi.AP.enableNAPT(true); // Habilita el NAPT, necesario para que comparta el internet del Wi-Fi
					
					TRACE("Abre en el navegador <http://%s> para Wi-Fi\n", WiFi.localIP().toString().c_str());
				}
			}
			else WiFi.disconnect();
			
			if (Jdoc["AP"]["enable"] == true) {
				TRACE("%s", (WiFi.status() == WL_CONNECTED) ? "o\n" : "");
				
				if (Jdoc["AP"]["dhcp"] == false) WiFi.softAPConfig(toIPAddress(AP_addresses["address"]), toIPAddress(AP_addresses["gateway"]), toIPAddress(AP_addresses["subnet"]), toIPAddress(AP_addresses["since"]), toIPAddress(AP_addresses["dns"]));
				
				WiFi.softAPsetHostname(HOSTNAME);
				WiFi.softAP((const char*)AP_credentials["ssid"], (const char*)AP_credentials["pass"]); // Iniciar la conexión del AP
				
				TRACE("Abre en el navegador <http://%s> para AP\n", WiFi.softAPIP().toString().c_str());
			}
			else WiFi.softAPdisconnect(true);
		}
	}
}

std::map<byte, IPAddress> clientMap; // Almacena las direcciones ip en las conexiones
#define clientPC ".141"

void initServer() { // Inicia el servidor

	server.enableCORS(true); // enable CORS header in webserver results
	
	for (int i = 0; i < dirFile.size(); i++){
		
		server.on(dirFile[i], [i]() {
			server.send(200, typeFile(dirFile[i]), readFile(dirFile[i])); // serve a built-in html page
		});
		
		if (String(dirFile[i]).endsWith("index.html")) \
			// This will redirect to the file index.html when it is existing otherwise to the built-in /index.html page
			server.on("/", HTTP_GET, [i]() {
				if (WiFi.status() == WL_CONNECTED) TRACE("Redirigiendo a <http://%s%s>\n", WiFi.localIP().toString().c_str(), dirFile[i]);
				else TRACE("Redirigiendo a <http://%s%s>\n", WiFi.softAPIP().toString().c_str(), dirFile[i]);

				server.sendHeader("Location", dirFile[i], true);
				server.send(302);
			});
		else if (String(dirFile[i]).endsWith("404_error.html")) \
			server.onNotFound([i]() {
				// standard not found in browser.
				TRACE("Fuente no encontrada...\n");
				server.send(404, typeFile(dirFile[i]), readFile(dirFile[i]));
			});
	}
	
	server.begin(); // start WebServer
	webSocket.begin(); // start WebSocket

	webSocket.onEvent([] (byte num, WStype_t type, uint8_t* payload, size_t length) {      // the parameters of this callback function are always the same -> num: id of the client who send the event, type: type of message, payload: actual data sent and length: length of payload
	 switch (type) {                                     // switch on the type of information sent
			case WStype_DISCONNECTED:                         // if a client is disconnected, then type == WStype_DISCONNECTED
				if (clientMap.find(num) != clientMap.end()) {
					TRACE("Cliente %s desconectado", clientMap[num].toString().c_str());
					clientMap.erase(num);
				}
				break;
			case WStype_CONNECTED:                            // if a client is connected, then type == WStype_CONNECTED
				clientMap[num] = webSocket.remoteIP(num);
				TRACE("Cliente %s conectado", clientMap[num].toString().c_str());
				break;
			case WStype_TEXT:                                 // if a client has sent data, then type == WStype_TEXT
				// try to decipher the JSON string received
				DeserializationError error = deserializeJson(Jdoc, payload);
				if (error) {
					TRACE(F("deserializeJson() fallido: %s"), error.f_str());
					return;
				}
				else {
					JsonObject PowerStatus = Jdoc["PowerStatus"];

					if (clientMap[num].toString().endsWith(clientPC)) modJFile("connected", PowerStatus["connected"]); // Cambia el valor de 'connected'
					else {
						// serializeJsonPretty(Jdoc, Serial); // Print the result
						
						bool change = false;

						for (int WA = 0; WA < 2; WA++){
							std::vector<std::vector<String>> JdocType = {
								{ // enable
									"enable"
								},
								{ // dhcp
									"dhcp"
								},
								{ // credentials
									"ssid",
									"pass"
								},
								{ // addresses
									"address",
									"gateway",
									"subnet"
								}
							};
							const String tNet = (WA == 0) ? "WiFi" : "AP";
							
							if (tNet == "WiFi") for (int dns = 1; dns <= 2; dns++) JdocType[3].push_back("dns" + String(dns));
							else for (int type = 1; type <= 2; type++) JdocType[3].push_back((type == 1) ? "since" : "dns");
							
							for (int i = 0; i < JdocType.size(); i++) {
								for (int j = 0; j < JdocType[i].size(); j++) {
									String JdocMod;
									if (i <= 1) JdocMod = (String)Jdoc[tNet][JdocType[i][j]]; // Se utiliza (String) para obtener true o false respectivamente, debido a que (const char*) o (bool) no devuelven el valor esperado para ser modificado
									else JdocMod = (String)Jdoc[tNet][(i == 2) ? "credentials" : "addresses"][JdocType[i][j]];
									
									// TRACE("%s\n", JdocMod.c_str());
									
									if (JdocMod != ((tNet == "WiFi") ? JdocWiFiOrg[i][j] : JdocAPOrg[i][j])) {
										change = true;
										modJFile(tNet.c_str(), JdocType[i][j].c_str(), (i <= 1) ? JdocMod : "\"" + JdocMod + "\""); // Cambia el valor de 'message'
									}
								}
							}
						}

						// Serial.println(readFile(dirFile[Jconfig]));

						if (change) {
							TRACE("Reiniciando\n");
							esp_restart(); // Reinicia el ESP32
						}
						else {
							modJFile("power", PowerStatus["power"]); // Cambia el valor de 'power'
							modJFile("message", "\"" + String(PowerStatus["message"]) + "\""); // Cambia el valor de 'message'

							// Enciende o apaga al pulsar el botón de la página en /index.html
							cont = 0;
							if (PowerStatus["power"] != 2) pressPower(P_NORMAL);
							else pressPower(P_FORCE);
							TRACE((const char*)PowerStatus["message"]);
						}
					}
				}
				break;
		}
		TRACE("\n");

		if (clientMap[num].toString().endsWith(clientPC)) {
			if (type == WStype_DISCONNECTED) connectedPC = false; // Si se desconectó el dispositivo se establece en false
			else if (type == WStype_CONNECTED) connectedPC = true; // Si se desconectó el dispositivo se establece en true
		} 
	}); // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"
}