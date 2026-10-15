#include "rng_int.h"
#include "velum/libk.h"
#include "velum/util.h"

void	rng_init(t_rng *r, const t_rng_env *env)
{
	memset(r, 0, sizeof(*r));
	spin_init(&r->lock, "random");
	r->env = env;
}

uint64_t	rng_now(const t_rng *r)
{
	if (!r->env->now_ns)
		return (0);
	return (r->env->now_ns());
}

static void	rng_mix(t_rng *r, const uint8_t *material, size_t n)
{
	t_sha256	h;
	t_sha256	snapshot;
	uint8_t		digest[SHA256_LEN];

	if (!r->ready)
		sha256_init(&r->pool);
	r->ready = 1;
	snapshot = r->pool;
	sha256_final(&snapshot, digest);
	sha256_init(&h);
	sha256_update(&h, "velum-rng-mix-v1", 16);
	sha256_update(&h, r->key, sizeof(r->key));
	sha256_update(&h, digest, sizeof(digest));
	sha256_update(&h, material, n);
	sha256_final(&h, r->key);
	sha256_init(&r->pool);
	sha256_update(&r->pool, digest, sizeof(digest));
	r->pool_bytes = 0;
	secure_zero(digest, sizeof(digest));
}

uint32_t	rng_reseed(t_rng *r)
{
	uint8_t		material[RNG_SEED_LEN];
	uint32_t	sources;
	uint64_t	now;
	uint64_t	flags;

	sources = rng_gather(r->env, material);
	now = rng_now(r);
	flags = spin_lock_irqsave(&r->lock);
	rng_mix(r, material, sizeof(material));
	r->since_reseed = 0;
	r->last_reseed_ns = now;
	r->seeded = 1;
	r->reseeds++;
	spin_unlock_irqrestore(&r->lock, flags);
	secure_zero(material, sizeof(material));
	return (sources);
}

void	rng_add(t_rng *r, const void *data, size_t n)
{
	const uint8_t	*p;
	uint64_t		flags;
	size_t			take;

	p = data;
	while (p && n)
	{
		take = min_u64(n, RNG_ADD_CHUNK);
		flags = spin_lock_irqsave(&r->lock);
		if (!r->ready)
			sha256_init(&r->pool);
		r->ready = 1;
		sha256_update(&r->pool, p, take);
		r->pool_bytes += take;
		spin_unlock_irqrestore(&r->lock, flags);
		p += take;
		n -= take;
	}
}
