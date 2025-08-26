// tweetnacl_wrapper.h
#ifndef TWEETNACL_WRAPPER_H
#define TWEETNACL_WRAPPER_H

#include <stdint.h>
#include <stddef.h>

// Key and nonce sizes for NaCl box
#define NACL_PUBLICKEY_SIZE 32
#define NACL_SECRETKEY_SIZE 32
#define NACL_NONCE_SIZE     24
#define NACL_BOX_OVERHEAD   16

#ifdef __cplusplus
extern "C" {
#endif

// Generates a keypair (public, secret)
int nacl_keypair(uint8_t *public_key, uint8_t *secret_key);

// Encrypts a message using public/secret key and nonce
int nacl_box(uint8_t *ciphertext, const uint8_t *message, uint64_t message_len,
             const uint8_t *nonce, const uint8_t *public_key, const uint8_t *secret_key);

// Decrypts a message using public/secret key and nonce
int nacl_box_open(uint8_t *message, const uint8_t *ciphertext, uint64_t cipher_len,
                  const uint8_t *nonce, const uint8_t *public_key, const uint8_t *secret_key);

// Fills a buffer with secure random bytes
int nacl_randombytes(uint8_t *buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif // TWEETNACL_WRAPPER_H
