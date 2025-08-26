#include <Arduino.h>
#include <esp_system.h>  // esp_fill_random()

extern "C" {
  #include "tweetnacl.h"
}

// Constants
#define NACL_PUBLICKEY_SIZE     32
#define NACL_SECRETKEY_SIZE     32
#define NACL_NONCE_SIZE         24
#define NACL_BOX_ZEROBYTES      32
#define NACL_BOX_BOXZEROBYTES   16

// Declare prototype
extern "C" int nacl_randombytes(uint8_t *buffer, size_t size);

// RNG
extern "C" int nacl_randombytes(uint8_t *buf, size_t len) {
  esp_fill_random(buf, len);
  return 0;
}

// Nonce
uint8_t nonce_[NACL_NONCE_SIZE];

// Hex printer
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
  delay(500);
  Serial.println("\nTweetNaCl Demo – Working Dynamic Keypairs");
  delay(1000);
  // Generate sender keypair
  uint8_t sender_pk[NACL_PUBLICKEY_SIZE];
  uint8_t sender_sk[NACL_SECRETKEY_SIZE];
  crypto_box_keypair(sender_pk, sender_sk);

  // Generate receiver keypair
  uint8_t receiver_pk[NACL_PUBLICKEY_SIZE];
  uint8_t receiver_sk[NACL_SECRETKEY_SIZE];
  crypto_box_keypair(receiver_pk, receiver_sk);

  // Generate fresh nonce
  nacl_randombytes(nonce_, NACL_NONCE_SIZE);
  
  Serial.println("Sender Public Key:"); print_hex(sender_pk, NACL_PUBLICKEY_SIZE);
  Serial.println("Receiver Public Key:"); print_hex(receiver_pk, NACL_PUBLICKEY_SIZE);
  Serial.println("Nonce:"); print_hex(nonce_, NACL_NONCE_SIZE);

  Serial.println("Sender Secret Key:");
  print_hex(sender_sk, NACL_SECRETKEY_SIZE);
  Serial.println("Receiver Secret Key:");
  print_hex(receiver_sk, NACL_SECRETKEY_SIZE);
  delay(1000);
  // Message
  const char *plaintext = "Secret: ESP32 to ESP32!";
  size_t msg_len = strlen(plaintext);
  size_t padded_len = msg_len + NACL_BOX_ZEROBYTES;

  // Pad message
  uint8_t *padded_msg = (uint8_t *)malloc(padded_len);
  memset(padded_msg, 0, NACL_BOX_ZEROBYTES);
  memcpy(padded_msg + NACL_BOX_ZEROBYTES, plaintext, msg_len);

  // Encrypt
  uint8_t *ciphertext = (uint8_t *)malloc(padded_len);
  if (crypto_box(ciphertext, padded_msg, padded_len, nonce_, receiver_pk, sender_sk) != 0) {
    Serial.println("Encryption failed!");
    return;
  }

  Serial.println("Ciphertext:");
  print_hex(ciphertext + NACL_BOX_BOXZEROBYTES, padded_len - NACL_BOX_BOXZEROBYTES);

  // Prepare padded ciphertext for decryption
  uint8_t *padded_cipher = (uint8_t *)malloc(padded_len);
  memset(padded_cipher, 0, NACL_BOX_BOXZEROBYTES);
  memcpy(padded_cipher + NACL_BOX_BOXZEROBYTES,
         ciphertext + NACL_BOX_BOXZEROBYTES,
         padded_len - NACL_BOX_BOXZEROBYTES);

  // Decrypt
  uint8_t *decrypted = (uint8_t *)malloc(padded_len);
  if (crypto_box_open(decrypted, padded_cipher, padded_len, nonce_, sender_pk, receiver_sk) != 0) {
    Serial.println("Decryption failed!");
    return;
  }

  Serial.print("Decrypted message: ");
  for (size_t i = 0; i < msg_len; i++) {
    Serial.print((char)decrypted[NACL_BOX_ZEROBYTES + i]);
  }
  Serial.println();

  free(padded_msg);
  free(ciphertext);
  free(padded_cipher);
  free(decrypted);
}

void loop() {
  delay(5000);
}
