#include <mcci_tweetnacl_stream.h>
#include <mcci_tweetnacl_sign.h>
#include <mcci_tweetnacl_secretbox.h>
#include <mcci_tweetnacl_scalarmult.h>
#include <mcci_tweetnacl_onetimeauth.h>
#include <mcci_tweetnacl_hash.h>
#include <mcci_tweetnacl_hal.h>
#include <mcci_tweetnacl_box.h>
#include <mcci_tweetnacl_auth.h>
#include <mcci_tweetnacl.h>
#include <Arduino.h>


void printHex(const char* label, const uint8_t* data, size_t len) {
  Serial.print(label);
  for (size_t i = 0; i < len; ++i) {
    char buf[4];
    sprintf(buf, "%02X ", data[i]);
    Serial.print(buf);
  }
  Serial.println();
}

void runEncryptionDecryption() {
  mcci_tweetnacl_secretbox_key_t key = {{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
  }};
  mcci_tweetnacl_secretbox_nonce_t nonce = {{
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7
  }};

  const char* msg = "run:blink_led";
  size_t msgLen = strlen(msg);
  size_t paddedLen = msgLen + 32;

  uint8_t* msgPadded = (uint8_t*)calloc(1, paddedLen);
  uint8_t* cipher = (uint8_t*)calloc(1, paddedLen);
  uint8_t* decrypted = (uint8_t*)calloc(1, paddedLen + 1); // +1 for \0

  if (!msgPadded || !cipher || !decrypted) {
    Serial.println("Memory allocation failed!");
    return;
  }

  memcpy(msgPadded + 32, msg, msgLen);

  int rc = mcci_tweetnacl_secretbox(cipher, msgPadded, paddedLen, &nonce, &key);
  if (rc != 0) {
    Serial.println("Encryption failed.");
    goto cleanup;
  }

  printHex("Ciphertext:", cipher + 16, msgLen + 16);

  rc = mcci_tweetnacl_secretbox_open(decrypted, cipher, paddedLen, &nonce, &key);
  if (rc != 0) {
    Serial.println("Decryption failed.");
    goto cleanup;
  }

  decrypted[32 + msgLen] = '\0';

  printHex("Decrypted:", decrypted + 32, msgLen);
  Serial.print("Decrypted String: ");
  Serial.println((const char*)(decrypted + 32));

cleanup:
  free(msgPadded);
  free(cipher);
  free(decrypted);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting TweetNaCl test loop...");
}

void loop() {
  runEncryptionDecryption();
  delay(5000); // every 5 seconds
}
