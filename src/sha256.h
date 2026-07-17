/*
 * sha256.h — SHA-256 hash. Public domain.
 * Based on Brad Conte's implementation:
 *   https://github.com/B-Con/crypto-algorithms
 */

#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>

#define SHA256_BLOCK_SIZE 32  /* SHA256 outputs a 32 byte digest */

typedef struct {
	unsigned char data[64];
	unsigned int  datalen;
	unsigned long long bitlen;
	unsigned int  state[8];
} SHA256_CTX;

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const unsigned char data[], size_t len);
void sha256_final(SHA256_CTX *ctx, unsigned char hash[]);

#endif
