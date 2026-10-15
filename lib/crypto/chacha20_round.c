#include "crypto_int.h"

static uint32_t	chacha20_rotl(uint32_t x, uint32_t n)
{
	return ((x << n) | (x >> (32 - n)));
}

static void	chacha20_qr(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
	*a += *b;
	*d = chacha20_rotl(*d ^ *a, 16);
	*c += *d;
	*b = chacha20_rotl(*b ^ *c, 12);
	*a += *b;
	*d = chacha20_rotl(*d ^ *a, 8);
	*c += *d;
	*b = chacha20_rotl(*b ^ *c, 7);
}

void	chacha20_double_round(uint32_t x[16])
{
	chacha20_qr(&x[0], &x[4], &x[8], &x[12]);
	chacha20_qr(&x[1], &x[5], &x[9], &x[13]);
	chacha20_qr(&x[2], &x[6], &x[10], &x[14]);
	chacha20_qr(&x[3], &x[7], &x[11], &x[15]);
	chacha20_qr(&x[0], &x[5], &x[10], &x[15]);
	chacha20_qr(&x[1], &x[6], &x[11], &x[12]);
	chacha20_qr(&x[2], &x[7], &x[8], &x[13]);
	chacha20_qr(&x[3], &x[4], &x[9], &x[14]);
}
