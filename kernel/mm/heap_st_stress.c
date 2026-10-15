#include "heap_int.h"
#include "heap_st.h"
#include "velum/libk.h"

static uint32_t	stress_size(t_st_rng *r)
{
	uint64_t	roll;

	roll = st_rand(r) % 100;
	if (roll < 70)
		return (1 + (uint32_t)(st_rand(r) % 2048));
	if (roll < 95)
		return (2049 + (uint32_t)(st_rand(r) % 18000));
	return (20001 + (uint32_t)(st_rand(r) % 180000));
}

static int	stress_alloc(t_st_state *s, uint32_t j)
{
	s->n[j] = stress_size(&s->rng);
	s->p[j] = kmalloc_tag(s->n[j], (t_heap_tag)(st_rand(&s->rng) % HEAP_TAGS));
	if (!s->p[j])
		return (1);
	st_fill(s->p[j], s->n[j], (uint8_t)j);
	return (0);
}

static int	stress_resize(t_st_state *s, uint32_t j)
{
	uint32_t	to;
	void		*q;

	to = stress_size(&s->rng);
	q = krealloc(s->p[j], to);
	if (!q)
		return (1);
	if (st_verify(q, min_u64(s->n[j], to), (uint8_t)j))
		return (2);
	s->p[j] = q;
	s->n[j] = to;
	st_fill(q, to, (uint8_t)j);
	return (0);
}

static int	stress_step(t_st_state *s)
{
	uint32_t	j;

	j = (uint32_t)(st_rand(&s->rng) % ST_SLOTS);
	if (!s->p[j])
		return (stress_alloc(s, j));
	if (st_verify(s->p[j], s->n[j], (uint8_t)j))
		return (3);
	if (st_rand(&s->rng) & 1)
		return (stress_resize(s, j));
	kfree(s->p[j]);
	s->p[j] = NULL;
	return (0);
}

int	st_stress(void)
{
	t_st_state	s;
	uint32_t	i;
	int			rc;

	memset(&s, 0, sizeof(s));
	s.rng.state = ST_SEED;
	rc = 0;
	i = 0;
	while (i < ST_OPS && rc == 0)
	{
		rc = stress_step(&s);
		if (i % 500 == 499 && heap_check())
			rc = 9;
		i++;
	}
	i = 0;
	while (i < ST_SLOTS)
		kfree(s.p[i++]);
	return (rc);
}
