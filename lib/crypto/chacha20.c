#include "chacha20.h"
#include "crypto_int.h"

void	chacha20_init(t_chacha20 *c, const uint8_t key[CHACHA20_KEY_LEN],
		const uint8_t nonce[CHACHA20_NONCE_LEN], uint32_t counter)
{
	uint32_t	i;

	c->state[0] = 0x61707865;
	c->state[1] = 0x3320646e;
	c->state[2] = 0x79622d32;
	c->state[3] = 0x6b206574;
	i = 0;
	while (i < 8)
	{
		c->state[4 + i] = crypto_load_le32(key + i * 4);
		i++;
	}
	c->state[12] = counter;
	i = 0;
	while (i < 3)
	{
		c->state[13 + i] = crypto_load_le32(nonce + i * 4);
		i++;
	}
}

void	chacha20_block(t_chacha20 *c, uint8_t out[CHACHA20_BLOCK_LEN])
{
	uint32_t	x[16];
	uint32_t	i;

	i = 0;
	while (i < 16)
	{
		x[i] = c->state[i];
		i++;
	}
	i = 0;
	while (i < 10)
	{
		chacha20_double_round(x);
		i++;
	}
	i = 0;
	while (i < 16)
	{
		crypto_store_le32(out + i * 4, x[i] + c->state[i]);
		i++;
	}
	c->state[12]++;
	secure_zero(x, sizeof(x));
}
