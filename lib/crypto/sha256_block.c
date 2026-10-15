#include "crypto_int.h"
#include "sha256_int.h"

static const uint32_t	g_k[64] = {
	0x428a2f98, 0x71374491, 0x0b5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
	0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
	0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0x0b00327c8, 0x0bf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
	0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0x0bef9a3f7, 0xc67178f2
};

static void	sha256_schedule(uint32_t w[64], const uint8_t *b)
{
	uint32_t	i;

	i = 0;
	while (i < 16)
	{
		w[i] = crypto_load_be32(b + i * 4);
		i++;
	}
	while (i < 64)
	{
		w[i] = sha_small(w[i - 2], 17, 19, 10) + w[i - 7]
			+ sha_small(w[i - 15], 7, 18, 3) + w[i - 16];
		i++;
	}
}

static void	sha256_round(uint32_t s[8], uint32_t kw)
{
	uint32_t	t1;
	uint32_t	t2;

	t1 = s[7] + sha_big(s[4], 6, 11, 25) + sha_ch(s[4], s[5], s[6]) + kw;
	t2 = sha_big(s[0], 2, 13, 22) + sha_maj(s[0], s[1], s[2]);
	s[7] = s[6];
	s[6] = s[5];
	s[5] = s[4];
	s[4] = s[3] + t1;
	s[3] = s[2];
	s[2] = s[1];
	s[1] = s[0];
	s[0] = t1 + t2;
}

void	sha256_compress(uint32_t h[8], const uint8_t block[SHA256_BLOCK])
{
	uint32_t	w[64];
	uint32_t	s[8];
	uint32_t	i;

	sha256_schedule(w, block);
	i = 0;
	while (i < 8)
	{
		s[i] = h[i];
		i++;
	}
	i = 0;
	while (i < 64)
	{
		sha256_round(s, g_k[i] + w[i]);
		i++;
	}
	i = 0;
	while (i < 8)
	{
		h[i] += s[i];
		i++;
	}
	secure_zero(w, sizeof(w));
	secure_zero(s, sizeof(s));
}
