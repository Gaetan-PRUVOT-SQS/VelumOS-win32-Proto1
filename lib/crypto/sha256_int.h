#ifndef SHA256_INT_H
# define SHA256_INT_H

# include <stdint.h>

static inline uint32_t	sha_rotr(uint32_t x, uint32_t n)
{
	return ((x >> n) | (x << (32 - n)));
}

static inline uint32_t	sha_ch(uint32_t x, uint32_t y, uint32_t z)
{
	return ((x & y) ^ (~x & z));
}

static inline uint32_t	sha_maj(uint32_t x, uint32_t y, uint32_t z)
{
	return ((x & y) ^ (x & z) ^ (y & z));
}

static inline uint32_t	sha_big(uint32_t x, uint32_t a, uint32_t b, uint32_t c)
{
	return (sha_rotr(x, a) ^ sha_rotr(x, b) ^ sha_rotr(x, c));
}

static inline uint32_t	sha_small(uint32_t x, uint32_t a, uint32_t b,
		uint32_t c)
{
	return (sha_rotr(x, a) ^ sha_rotr(x, b) ^ (x >> c));
}

#endif
