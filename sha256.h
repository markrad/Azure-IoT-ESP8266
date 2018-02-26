/* public domain sha256 implementation based on fips180-3 */

/*
 * Adapted from code from https://github.com/mikejsavage/hmac
 */
 
#pragma once

struct sha256 {
	uint64_t len;    /* processed message length */
	uint32_t h[8];   /* hash state */
	uint8_t buf[64]; /* message block buffer */
};

enum { SHA256_DIGEST_LENGTH = 32 };

/* reset state */
void sha256Init(void *ctx);
/* process message */
void sha256Update(void *ctx, const void *m, unsigned long len);
/* get message digest */
/* state is ruined after sum, keep a copy if multiple sum is needed */
/* part of the message might be left in s, zero it if secrecy is needed */
void sha256Sum(void *ctx, uint8_t md[SHA256_DIGEST_LENGTH]);

#ifdef __cplusplus
extern "C"
{
#endif
  /* Wraps up all of the hash generation logic in a C++ accessible function 
   *  
   *  hashedDataOut:    Hashed data will be copied here. This must be SHA256_DIGEST_LENGTH or memory corruption will occur 
   *  data:             Data to be hashed
   *  dataLen:          Length of data to be hashed
   *  keyInput          Key to use for hashing
   *  keyInputLength    Length of key to use for hashing
   *  
   * Returns 0 for success otherwise the parameters are in error
   */
  int generateHash(uint8_t *hashedDataOut, uint8_t *data, size_t dataLen, uint8_t *keyInput, size_t keyInputLen);
#ifdef __cplusplus
}
#endif


