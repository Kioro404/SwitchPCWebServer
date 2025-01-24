#define TRACE(...) Serial.printf(__VA_ARGS__) // TRACE output simplified, can be deactivated here

#define PORT 80 // puerto de conexión

#define FORMAT_LITTLEFS_IF_FAILED true

#define P_NORMAL 2250
#define P_FORCE 5000

WebServer server(PORT); // need a WebServer for http access on port 80.
WebSocketsServer webSocket = WebSocketsServer(PORT + 1);    // the websocket uses port 81 (standard port for websockets