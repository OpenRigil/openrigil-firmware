// from canokey-crypto
#include "rsa.h"

#if MMM_BLOCK == 64

int rsa2048_get_public_key(rsa_key_t *key, uint8_t *n);
int rsa4096_get_public_key(rsa_key_t *key, uint8_t *n);

int rsa_get_public_key(rsa_key_t *key, uint8_t *n) {
    if (key->nbits == 2048) {
        return rsa2048_get_public_key(key, n);
    } else if (key->nbits == 4096) {
        return rsa4096_get_public_key(key, n);
    }
    return 0;
}

int rsa2048_private(rsa_key_t *key, const uint8_t *input, uint8_t *output);
int rsa4096_private(rsa_key_t *key, const uint8_t *input, uint8_t *output);

int rsa_private(rsa_key_t *key, const uint8_t *input, uint8_t *output) {
    if (key->nbits == 2048) {
        return rsa2048_private(key, input, output);
    } else if (key->nbits == 4096) {
        return rsa4096_private(key, input, output);
    }
    return 0;
}

#endif
