#include <stdio.h>
#include "fake_f4.h"
#include "fake_sys.h"
#include "velum/err.h"

static const uint64_t	g_special[12] = {0, 1, 2, 3, 6, 25, 256, 257, 0x7001,
	0x7002, UINT64_MAX, 0x8000000000000000ull};
static int				g_hist[5];

static int	f4_slot(int rc)
{
	if (rc == START_OK)
		return (0);
	if (rc == START_TRUNCATED)
		return (1);
	if (rc == E_INVAL)
		return (2);
	if (rc == E_RANGE)
		return (3);
	if (rc == E_PROTO)
		return (4);
	return (-1);
}

void	f4_fill_random(t_f4_stack *s, uint64_t *rng, int structural)
{
	size_t		i;
	uint64_t	r;

	s->n = f4_rng(rng) % 65;
	i = 0;
	while (i < s->n)
	{
		r = f4_rng(rng);
		if (structural && (r >> 60) < 12)
			s->w[i] = g_special[(r >> 56) % 12];
		else
			s->w[i] = r;
		i++;
	}
}

void	f4_fill_mutated(t_f4_stack *s, uint64_t *rng)
{
	uint64_t	r;
	size_t		total;

	r = f4_rng(rng);
	f4_reset(s);
	f4_head(s, r % 4, (r >> 8) % 3);
	f4_pairs(s, (r >> 16) % 8, 0x1234);
	f4_push(s, AT_NULL);
	f4_push(s, 0);
	total = s->n;
	s->n = f4_rng(rng) % (total + 1);
	r = f4_rng(rng);
	if (s->n && (r & 1))
		s->w[(r >> 8) % s->n] = g_special[(r >> 32) % 12];
}

int	f4_fuzz_one(const t_f4_stack *s)
{
	t_startinfo		info;
	const uint64_t	*buf;
	int				rc;
	int				ok;

	buf = f4_open(s, s->n);
	rc = start_parse(buf, s->n, &info);
	ok = rc == f4_model(s->w, s->n) && f4_slot(rc) >= 0;
	if (ok && rc >= 0)
		ok = (const uint64_t *)info.argv == buf + 1
			&& (const uint64_t *)info.envp == buf + 2 + info.argc;
	f4_close(buf, s->n);
	if (!ok)
		fprintf(stderr, "fuzz : %zu mots, code %d, modele %d\n", s->n, rc,
			f4_model(s->w, s->n));
	else
		g_hist[f4_slot(rc)]++;
	return (!ok);
}

int	f4_hist(int slot)
{
	return (g_hist[slot]);
}
