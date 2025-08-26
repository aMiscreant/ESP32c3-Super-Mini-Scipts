#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Crypto.h>
#include <AES.h>
#include <CTR.h>
#include <ArduinoJson.h>
#include <base64.h>

const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWORD_HERE";
const char* serverAddress = "192.168.66.107";
const int serverPort = 8000;
const char* deviceID = "ESP32_1";

WebSocketsClient webSocket;

// AES-256 key (must match FastAPI server)
const byte aesKey[32] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

// Generate 16-byte nonce
void generateNonce(byte* nonce, size_t len = 16) {
  for (size_t i = 0; i < len; ++i) {
    nonce[i] = random(0, 256);
  }
}

// Encrypt plaintext using AES-256 CTR and return JSON string
String encryptMessage(const String& plaintext) {
  byte nonce[16];
  generateNonce(nonce);

  CTR<AES256> ctr;
  ctr.clear();
  ctr.setKey(aesKey, sizeof(aesKey));
  ctr.setIV(nonce, sizeof(nonce));

  size_t len = plaintext.length();
  byte plainBytes[len];
  memcpy(plainBytes, plaintext.c_str(), len);

  byte cipherBytes[len];
  ctr.encrypt(cipherBytes, plainBytes, len);

  String nonce_b64 = base64::encode(nonce, sizeof(nonce));
  String cipher_b64 = base64::encode(cipherBytes, len);

  DynamicJsonDocument doc(512);
  doc["nonce"] = nonce_b64;
  doc["ciphertext"] = cipher_b64;

  String json;
  serializeJson(doc, json);
  return json;
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_TEXT:
      Serial.println("[Server] " + String((char*)payload));
      break;

    case WStype_CONNECTED: {
      Serial.println("[+] WebSocket connected.");
      String payload = encryptMessage("PING");
      webSocket.sendTXT(payload);
      Serial.println(payload);
      break;
    }

    case WStype_DISCONNECTED:
      Serial.println("[-] WebSocket disconnected.");
      break;
  }
}

void connectWiFi() {
  Serial.print("[*] Connecting to WiFi...");
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println(WiFi.localIP());
}

void connectWebSocket() {
  String wsPath = String("/ws/") + deviceID;
  webSocket.begin(serverAddress, serverPort, wsPath.c_str());
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Optional: give serial monitor time
  connectWiFi();
  connectWebSocket();
}

void loop() {
  webSocket.loop();
}
