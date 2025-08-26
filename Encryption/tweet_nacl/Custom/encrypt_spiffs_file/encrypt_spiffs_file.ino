#include <Arduino.h>
#include <esp_system.h>     // esp_fill_random()
#include <FS.h>
#include <SPIFFS.h>

extern "C" {
  #include "tweetnacl.h"
}

// Constants
#define NACL_PUBLICKEY_SIZE     32
#define NACL_SECRETKEY_SIZE     32
#define NACL_NONCE_SIZE         24
#define NACL_BOX_ZEROBYTES      32
#define NACL_BOX_BOXZEROBYTES   16

extern "C" int nacl_randombytes(uint8_t *buf, size_t len) {
  esp_fill_random(buf, len);
  return 0;
}

uint8_t nonce_[NACL_NONCE_SIZE];

// Print hex
void print_hex(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (data[i] < 0x10) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nTweetNaCl Demo – SPIFFS File Encryption");
  delay(1000);
  // Mount SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Generate dynamic keypairs
  uint8_t sender_pk[NACL_PUBLICKEY_SIZE];
  uint8_t sender_sk[NACL_SECRETKEY_SIZE];
  crypto_box_keypair(sender_pk, sender_sk);

  uint8_t receiver_pk[NACL_PUBLICKEY_SIZE];
  uint8_t receiver_sk[NACL_SECRETKEY_SIZE];
  crypto_box_keypair(receiver_pk, receiver_sk);

  nacl_randombytes(nonce_, NACL_NONCE_SIZE);

  Serial.println("Sender Public Key:"); print_hex(sender_pk, NACL_PUBLICKEY_SIZE);
  Serial.println("Receiver Public Key:"); print_hex(receiver_pk, NACL_PUBLICKEY_SIZE);
  Serial.println("Nonce:"); print_hex(nonce_, NACL_NONCE_SIZE);
  delay(1000);
  // -------------------------------------------------
  // STEP 1 – Write plaintext file
  const char *message = "Secret message from ESP32 SPIFFS!";
  File f = SPIFFS.open("/secret.txt", FILE_WRITE);
  if (!f) {
    Serial.println("Failed to open file for writing");
    return;
  }
  f.write((const uint8_t *)message, strlen(message));
  f.close();
  Serial.println("Plaintext written to /secret.txt");

  // -------------------------------------------------
  // STEP 2 – Read plaintext file
  f = SPIFFS.open("/secret.txt", FILE_READ);
  if (!f) {
    Serial.println("Failed to open file for reading");
    return;
  }
  size_t plaintext_len = f.size();
  Serial.print("Plaintext file size: ");
  Serial.println(plaintext_len);

  uint8_t *plaintext = (uint8_t *)malloc(plaintext_len);
  f.read(plaintext, plaintext_len);
  f.close();

  // -------------------------------------------------
  // STEP 3 – Pad plaintext
  size_t padded_len = plaintext_len + NACL_BOX_ZEROBYTES;
  uint8_t *padded_msg = (uint8_t *)malloc(padded_len);
  memset(padded_msg, 0, NACL_BOX_ZEROBYTES);
  memcpy(padded_msg + NACL_BOX_ZEROBYTES, plaintext, plaintext_len);

  // -------------------------------------------------
  // STEP 4 – Encrypt
  uint8_t *ciphertext = (uint8_t *)malloc(padded_len);
  if (crypto_box(ciphertext, padded_msg, padded_len, nonce_, receiver_pk, sender_sk) != 0) {
    Serial.println("Encryption failed!");
    return;
  }
  Serial.println("Ciphertext generated!");

  // -------------------------------------------------
  // STEP 5 – Write ciphertext to SPIFFS
  File encFile = SPIFFS.open("/secret.enc", FILE_WRITE);
  if (!encFile) {
    Serial.println("Failed to open file for writing ciphertext");
    return;
  }
  // Save only the meaningful part, without the leading box-zero bytes
  encFile.write(ciphertext + NACL_BOX_BOXZEROBYTES, padded_len - NACL_BOX_BOXZEROBYTES);
  encFile.close();
  Serial.println("Ciphertext written to /secret.enc");

  // -------------------------------------------------
  // STEP 6 – Read ciphertext for decryption
  encFile = SPIFFS.open("/secret.enc", FILE_READ);
  if (!encFile) {
    Serial.println("Failed to open /secret.enc for reading");
    return;
  }

  size_t enc_size = encFile.size();
  Serial.print("Encrypted file size: ");
  Serial.println(enc_size);

  uint8_t *ciphertext_from_file = (uint8_t *)malloc(enc_size + NACL_BOX_BOXZEROBYTES);
  memset(ciphertext_from_file, 0, NACL_BOX_BOXZEROBYTES);
  encFile.read(ciphertext_from_file + NACL_BOX_BOXZEROBYTES, enc_size);
  encFile.close();

  // -------------------------------------------------
  // STEP 5b – Read and print the encrypted file
  encFile = SPIFFS.open("/secret.enc", FILE_READ);
  if (!encFile) {
    Serial.println("Failed to open /secret.enc for reading");
    return;
  }

  Serial.println("Ciphertext file contents:");

  while (encFile.available()) {
    uint8_t b = encFile.read();
    if (b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
  Serial.println();

  encFile.close();
  Serial.println();
  // -------------------------------------------------
  // STEP 7 – Decrypt
  uint8_t *decrypted = (uint8_t *)malloc(padded_len);
  if (crypto_box_open(decrypted, ciphertext_from_file, padded_len, nonce_, sender_pk, receiver_sk) != 0) {
    Serial.println("❌ Decryption failed!");
    return;
  }

  Serial.print("Decrypted message: ");
  for (size_t i = 0; i < plaintext_len; i++) {
    Serial.print((char)decrypted[NACL_BOX_ZEROBYTES + i]);
  }
  Serial.println();

  free(plaintext);
  free(padded_msg);
  free(ciphertext);
  free(ciphertext_from_file);
  free(decrypted);
}

void loop() {
  delay(10000);
}
