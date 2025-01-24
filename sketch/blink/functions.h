const int Contact1 = 2;   // Pin GPIO2
const int Contact2 = 13;  // Pin GPIO13

void pressPower(int time) {
	digitalWrite(Contact1, HIGH);
	delay(time);
	digitalWrite(Contact1, LOW);
}

IPAddress toIPAddress(const char* ipString) {
	// Dividir la cadena en octetos
	int octet1, octet2, octet3, octet4;
	sscanf(ipString, "%d.%d.%d.%d", &octet1, &octet2, &octet3, &octet4);

	// TRACE("%s\n", IPAddress(octet1, octet2, octet3, octet4).toString().c_str());
	return IPAddress(octet1, octet2, octet3, octet4);
}

const char* typeFile(String file) {

	if (file.endsWith(".html")) return "text/html";
	else if (file.endsWith(".css")) return "text/css";
	else if (file.endsWith(".js")) return "application/javascript";
	else if (file.endsWith(".json")) return "application/json";
	else if (file.endsWith(".svg")) return "image/svg+xml";

	return nullptr;
}

static std::vector<const char*> dirFile;
int Jconfig;

void listDir(const char* dirname, uint8_t levels) {
	TRACE("%s\n", dirname);
	
	File root = LittleFS.open(dirname);
	if (!root) TRACE("Fallo al abrir el directorio\n");
	else if (!root.isDirectory()) TRACE("El directorio no existe\n");
	else {
		File file = root.openNextFile();
		while (file) {
			if (file.isDirectory()) {
				TRACE("└─ %s\n", file.path());
				if (levels) listDir(file.path(), levels - 1);
			}
			else {
				TRACE("└─ %s %u bytes\n", file.name(), file.size());
				if (typeFile(file.path()) != nullptr) {
					
					const char* path = new char[strlen(file.path()) + 1];
					strcpy(const_cast<char*>(path), file.path());
					dirFile.push_back(path); // Almacenar el puntero a la nueva memoria
					
					if (String(dirFile.back()).endsWith("config.json")) Jconfig = dirFile.size() - 1;
				}
			}
			file = root.openNextFile();
		}
	}
}

// Convierte la información dentro de un archivo y lo almacena en un String
static const char* readFile(const char* path) {

	File file = LittleFS.open(path, "r");  // Abrir el archivo
	if (!file) TRACE("Fallo al abrir el archivo\n");
	else {
		const size_t size = file.size();  // Obtener el tamaño del archivo
		if (size == 0) TRACE("El archivo está vacio\n");
		else {
			char* buff = new char[size + 1];  // Asignar memoria para el buffer

			file.read((uint8_t*)buff, size);  // Leer el archivo completo en el buffer
			buff[size] = '\0';                // Añadir el terminador nulo
			file.close();

			// TRACE("%s\n", buff);
			return buff;
		}
	}

	file.close();
	return nullptr;
}

// Escribe información en un archivo
void writeFile(const char* path, String& content) {
	File file = LittleFS.open(path, "w");
	if (!file) {
		TRACE("Error al abrir el archivo para escritura\n");
		return;
	}

	file.print(content);
	file.close();
}

// Modifica el contenido de un archivo JSON
void modJFile(const char* key, const String& newValue) {
	static String jsonString = readFile(dirFile[Jconfig]);

	if (jsonString != nullptr) {
		// Buscar la clave en la cadena JSON
		const int keyIndex = jsonString.indexOf(String("\"") + key + "\":");
		if (keyIndex == -1) {
			// TRACE("Parametro no encontrado en el JSON\n");
			return;
		}

		// Calcular la posición inicial y final del valor
		const int startIndex = keyIndex + String("\"").length() + String(key).length() + String("\":").length();
		int endIndex = jsonString.indexOf(",", startIndex);
		if (endIndex == -1) endIndex = jsonString.indexOf("\n}", startIndex);

		// Reemplazar el valor antiguo con el nuevo valor
		if (jsonString[endIndex - 1] == '}') jsonString = jsonString.substring(0, startIndex) + " " + newValue + jsonString.substring(jsonString.indexOf("\n", startIndex));
		else jsonString = jsonString.substring(0, startIndex) + " " + newValue + jsonString.substring(endIndex);
		// Escribir la cadena modificada de vuelta al archivo JSON

		writeFile(dirFile[Jconfig], jsonString);

		server.on(dirFile[Jconfig], []() {
			server.send(200, typeFile(dirFile[Jconfig]), jsonString);  // serve a built-in html page
		});
	}
}

void modJFile(const char* key1, const char* key2, const String& newValue) {
	const String jsonString = readFile(dirFile[Jconfig]);

	if (jsonString != nullptr) {
		// Buscar la clave en la cadena JSON
		const int startIndex = jsonString.indexOf(String(key1) + "\":");

																							// endIndex
		modJFile(jsonString.substring(startIndex, jsonString.indexOf(String(key2) + "\":", startIndex) + strlen(key2)).c_str(), newValue);
	}
}