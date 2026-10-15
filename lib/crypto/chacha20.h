#ifndef CHACHA20_H
# define CHACHA20_H

# include <stdint.h>

# define CHACHA20_KEY_LEN 32
# define CHACHA20_NONCE_LEN 12
# define CHACHA20_BLOCK_LEN 64

typedef struct s_chacha20
{
	uint32_t	state[16];
}	t_chacha20;

void	chacha20_init(t_chacha20 *c, const uint8_t key[CHACHA20_KEY_LEN],
			const uint8_t nonce[CHACHA20_NONCE_LEN], uint32_t counter);
void	chacha20_block(t_chacha20 *c, uint8_t out[CHACHA20_BLOCK_LEN]);

#endif
