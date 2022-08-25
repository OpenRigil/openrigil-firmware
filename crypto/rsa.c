// from canokey-crypto
#include "rsa.h"

#if MMM_BLOCK == 64

int rsa2048_get_public_key(rsa_key_t *key, uint8_t *n);
int rsa4096_get_public_key(rsa_key_t *key, uint8_t *n);

// this does not work for now
//int rsa_get_public_key(rsa_key_t *key, uint8_t *n) {
//    if (key->nbits == 2048) {
//        rsa2048_get_public_key(key, n);
//        return 0;
//    } else if (key->nbits == 4096) {
//        rsa4096_get_public_key(key, n);
//        return 0;
//    }
//    return 0;
//}

int rsa2048_private(rsa_key_t *key, const uint8_t *input, uint8_t *output);
int rsa4096_private(rsa_key_t *key, const uint8_t *input, uint8_t *output);

int rsa_private(rsa_key_t *key, const uint8_t *input, uint8_t *output) {
    if (key->nbits == 2048) {
        rsa2048_private(key, input, output);
        return 0;
    } else if (key->nbits == 4096) {
        rsa4096_private(key, input, output);
        return 0;
    }
    return 0;
}

#endif
