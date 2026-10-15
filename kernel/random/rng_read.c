#include "rng_int.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "velum/util.h"

static const uint8_t	g_zero_nonce[CHACHA20_NONCE_LEN];

static int	reseed_due(const t_rng *r, uint64_t now)
{
	uint64_t	age;

	if (!r->seeded || r->since_reseed >= RNG_RESEED_BYTES)
		return (1);
	age = 0;
	if (now > r->last_reseed_ns)
		age = now - r->last_reseed_ns;
	if (age >= RNG_RESEED_NS)
		return (1);
	return (r->pool_bytes >= RNG_POOL_TRIGGER && age >= RNG_RESEED_EARLY_NS);
}

static int	chunk_take(t_rng *r, uint8_t out[CHACHA20_BLOCK_LEN], size_t take)
{
	t_chacha20	c;
	uint64_t	flags;

	flags = spin_lock_irqsave(&r->lock);
	if (!r->seeded)
	{
		spin_unlock_irqrestore(&r->lock, flags);
		return (E_AGAIN);
	}
	chacha20_init(&c, r->key, g_zero_nonce, 0);
	chacha20_block(&c, out);
	memcpy(r->key, out, CHACHA20_KEY_LEN);
	r->since_reseed += take;
	spin_unlock_irqrestore(&r->lock, flags);
	secure_zero(&c, sizeof(c));
	return (0);
}

static void	rng_refresh(t_rng *r)
{
	uint64_t	now;
	uint64_t	flags;
	int			due;

	now = rng_now(r);
	flags = spin_lock_irqsave(&r->lock);
	due = reseed_due(r, now);
	spin_unlock_irqrestore(&r->lock, flags);
	if (due)
		rng_reseed(r);
}

void	rng_read(t_rng *r, void *buf, size_t n)
{
	uint8_t	block[CHACHA20_BLOCK_LEN];
	uint8_t	*p;
	size_t	take;

	p = buf;
	if (!p || !n)
		return ;
	rng_refresh(r);
	while (n)
	{
		take = min_u64(n, RNG_CHUNK);
		while (chunk_take(r, block, take) == E_AGAIN)
			rng_reseed(r);
		memcpy(p, block + CHACHA20_KEY_LEN, take);
		p += take;
		n -= take;
	}
	secure_zero(block, sizeof(block));
}
