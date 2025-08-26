#include "WiFi.h"

// Flag to indicate whether we should scan Wi-Fi
bool wifiScan = false;

void showMenu() {
  Serial.println(F(" _________________________"));
  Serial.println(F("|     ESP32-C3 Super     |"));
  Serial.println(F("|      Mini Board        |"));
  Serial.println(F("|------------------------|"));
  Serial.println(F("| [ ] EN           IO10  |"));  
  Serial.println(F("| [ ] 3V3          IO6   |"));
  Serial.println(F("| [ ] IO0          IO7   |"));
  Serial.println(F("| [ ] GND          IO5   |"));
  Serial.println(F("| [ ] IO1          IO4   |"));
  Serial.println(F("| [ ] IO2          IO3   |"));
  Serial.println(F("| [ ] IO8          GND   |"));
  Serial.println(F("| [ ] IO9          5V    |"));
  Serial.println(F("|------------------------|"));
  Serial.println(F("|__USB-C________RST BTN__|"));
  Serial.println(F("")); 
  Serial.println(F("  [s]  Wi-Fi scan      "));
  Serial.println(F("  [m]  Show menu again "));
}

void handleMenuInput(char input) {
  switch (input) {
    case 's':
      wifiScan = true;
      break;
    case 'm':
      showMenu();
      break;
    default:
      Serial.println(F("? Unknown command. Type 'm' to show menu options."));
      break;
  }
}

void scanWifi() {
  Serial.println("\nStarting Wi-Fi Scan...");

  int n = WiFi.scanNetworks();
  Serial.println("Scan complete.");

  if (n == 0) {
    Serial.println("No networks found.");
  } else {
    Serial.printf("%d networks found:\n", n);
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");

    for (int i = 0; i < n; ++i) {
      Serial.printf("%2d | %-32.32s | %4d | %2d | ",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i),
                    WiFi.channel(i));

      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:           Serial.print("Open"); break;
        case WIFI_AUTH_WEP:            Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:        Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:       Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:   Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE:Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:       Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:  Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:       Serial.print("WAPI"); break;
        default:                       Serial.print("Unknown"); break;
      }

      Serial.println();
      delay(10);
    }
  }

  WiFi.scanDelete(); // free memory from scan result

  wifiScan = false; // reset flag
}

void setup() {
  Serial.begin(115200);

  // Set Wi-Fi to station mode and disconnect from any previous connection
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("WiFi Scanner Ready.");
  showMenu();
}

void loop() {
  if (Serial.available()) {
    char input = Serial.read();

    // Ignore newlines/carriage returns
    if (input != '\n' && input != '\r') {
      handleMenuInput(input);
    }

    // Clear the rest of the buffer
    while (Serial.available()) {
      Serial.read();
    }
  }

  if (wifiScan) {
    scanWifi();
  }
}
