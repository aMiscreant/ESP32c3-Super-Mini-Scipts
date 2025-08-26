#include "tweetnacl_wrapper.h"
#include "tweetnacl.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <user_interface.h>  // For os_random()

int nacl_randombytes(uint8_t *buffer, size_t size) {
    if (!buffer) return -1;
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = (uint8_t)os_random();
    }
    return 0;
}

int nacl_keypair(uint8_t *public_key, uint8_t *secret_key) {
    if (nacl_randombytes(secret_key, NACL_SECRETKEY_SIZE) != 0) {
        return -1;
    }

    if (crypto_scalarmult_base(public_key, secret_key) != 0) {
        return -1;
    }

    return 0;
}

int nacl_box(uint8_t *ciphertext, const uint8_t *message, uint64_t message_len,
             const uint8_t *nonce, const uint8_t *public_key, const uint8_t *secret_key) {
    uint8_t *padded_msg = (uint8_t *)malloc(message_len + NACL_BOX_OVERHEAD);
    if (!padded_msg) return -1;

    memset(padded_msg, 0, NACL_BOX_OVERHEAD);
    memcpy(padded_msg + NACL_BOX_OVERHEAD, message, message_len);

    int result = crypto_box(ciphertext, padded_msg, message_len + NACL_BOX_OVERHEAD, nonce, public_key, secret_key);
    free(padded_msg);
    return result;
}

int nacl_box_open(uint8_t *message, const uint8_t *ciphertext, uint64_t ciphertext_len,
                  const uint8_t *nonce, const uint8_t *public_key, const uint8_t *secret_key) {
    uint8_t *padded_ctext = (uint8_t *)malloc(ciphertext_len + NACL_BOX_OVERHEAD);
    if (!padded_ctext) return -1;

    memset(padded_ctext, 0, NACL_BOX_OVERHEAD);
    memcpy(padded_ctext + NACL_BOX_OVERHEAD, ciphertext, ciphertext_len);

    int result = crypto_box_open(message, padded_ctext, ciphertext_len + NACL_BOX_OVERHEAD, nonce, public_key, secret_key);
    free(padded_ctext);
    return result;
}
