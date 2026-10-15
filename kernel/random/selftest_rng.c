#include "rng_int.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/random.h"

static int	distinct_bytes(const uint8_t *buf, size_t n)
{
	uint8_t	seen[256];
	size_t	i;
	int		count;

	memset(seen, 0, sizeof(seen));
	count = 0;
	i = 0;
	while (i < n)
	{
		count += !seen[buf[i]];
		seen[buf[i]] = 1;
		i++;
	}
	return (count);
}

static int	output_checks(void)
{
	uint8_t	a[32];
	uint8_t	b[32];
	uint8_t	big[1024];
	uint8_t	zero[32];

	memset(zero, 0, sizeof(zero));
	krandom(a, sizeof(a));
	krandom(b, sizeof(b));
	if (!memcmp(a, b, sizeof(a)) || !memcmp(a, zero, sizeof(a)))
		return (1);
	krandom(big, sizeof(big));
	if (distinct_bytes(big, sizeof(big)) < 200)
		return (2);
	return (0);
}

static int	below_checks(void)
{
	uint64_t	hits;
	uint64_t	v;
	int			i;

	if (krandom_below(0) != 0 || krandom_below(1) != 0)
		return (3);
	hits = 0;
	i = 0;
	while (i < 1000)
	{
		v = krandom_below(7);
		if (v >= 7)
			return (4);
		hits |= 1ull << v;
		if (krandom_below(1000) >= 1000)
			return (5);
		i++;
	}
	if (hits != 0x7f)
		return (6);
	return (0);
}

static int	reseed_checks(void)
{
	t_rng		*r;
	uint64_t	before;
	uint64_t	a;
	uint64_t	b;

	r = rng_global();
	before = r->reseeds;
	a = krandom_u64();
	random_add_entropy("selftest", 8);
	rng_reseed(r);
	b = krandom_u64();
	if (r->reseeds != before + 1 || a == b)
		return (7);
	return (0);
}

int	rng_selftest(void)
{
	int	rc;

	rc = output_checks();
	if (!rc)
		rc = below_checks();
	if (!rc)
		rc = reseed_checks();
	if (rc)
		klog_err("random: controle du generateur %d en echec", rc);
	return (rc);
}
