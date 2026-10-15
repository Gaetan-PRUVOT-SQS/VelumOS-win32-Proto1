#include "rng_int.h"
#include "velum/err.h"

static int	hw_word(int (*fn)(uint64_t *), uint64_t *v)
{
	int	tries;
	int	rc;

	rc = E_NODEV;
	tries = 0;
	while (fn && tries < RNG_HW_RETRIES)
	{
		rc = fn(v);
		if (rc == 0 || rc == E_NODEV)
			return (rc);
		tries++;
	}
	return (rc);
}

static uint32_t	mix_hw(int (*fn)(uint64_t *), t_sha256 *h, uint32_t flag)
{
	uint64_t	v;
	uint32_t	got;
	uint32_t	i;

	got = 0;
	i = 0;
	while (i < RNG_HW_WORDS)
	{
		if (hw_word(fn, &v) == 0)
		{
			sha256_update(h, &v, sizeof(v));
			got = flag;
		}
		i++;
	}
	secure_zero(&v, sizeof(v));
	return (got);
}

static void	jitter_work(volatile uint32_t *sink)
{
	uint32_t	i;

	i = 0;
	while (i < 16)
	{
		*sink = *sink * 1103515245u + 12345u;
		i++;
	}
}

static uint32_t	mix_jitter(const t_rng_env *e, t_sha256 *h)
{
	volatile uint32_t	sink;
	uint64_t			t;
	uint32_t			i;

	if (!e->cycles)
		return (0);
	sink = 1;
	i = 0;
	while (i < RNG_JITTER_SAMPLES)
	{
		jitter_work(&sink);
		t = e->cycles();
		sha256_update(h, &t, sizeof(t));
		i++;
	}
	return (RNG_SRC_JITTER);
}

uint32_t	rng_gather(const t_rng_env *e, uint8_t out[RNG_SEED_LEN])
{
	t_sha256	h;
	uint64_t	now;
	uint32_t	src;

	sha256_init(&h);
	sha256_update(&h, "velum-rng-gather-v1", 19);
	src = mix_hw(e->rdseed, &h, RNG_SRC_RDSEED);
	src |= mix_hw(e->rdrand, &h, RNG_SRC_RDRAND);
	src |= mix_jitter(e, &h);
	now = 0;
	if (e->now_ns)
		now = e->now_ns();
	sha256_update(&h, &now, sizeof(now));
	if (now)
		src |= RNG_SRC_TIME;
	sha256_final(&h, out);
	return (src);
}
