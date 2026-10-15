#include <string.h>
#include "a04_fake.h"
#include "velum/util.h"

static void	meta_free(t_meta *m, uint32_t j)
{
	m->expect -= m->slot[j];
	m->live--;
	kfree(m->p[j]);
	m->p[j] = NULL;
}

static void	meta_resize(t_meta *m, uint32_t j)
{
	uint32_t	to;
	void		*q;

	to = a04_meta_size(&m->rng);
	q = krealloc(m->p[j], to);
	h_true(q != NULL, "meta: realloc");
	h_eq_i64("meta: prefixe conserve", a04_verify(q, min_u64(m->n[j], to),
			(uint8_t)j), 0);
	if (q != m->p[j])
	{
		m->expect -= m->slot[j];
		m->slot[j] = a04_slot(to);
		m->expect += m->slot[j];
	}
	m->p[j] = q;
	m->n[j] = to;
	m->sum = m->sum * 31 + ((uintptr_t)q - g_heap.lay.base);
	a04_fill(q, to, (uint8_t)j);
}

static void	meta_step(t_meta *m)
{
	uint32_t	j;

	j = (uint32_t)a04_rng_below(&m->rng, A04_META_SLOTS);
	if (!m->p[j])
		a04_meta_alloc(m, j);
	else
	{
		h_eq_i64("meta: contenu avant action", a04_verify(m->p[j], m->n[j],
				(uint8_t)j), 0);
		if (a04_rng_below(&m->rng, 100) < 55)
			meta_free(m, j);
		else
			meta_resize(m, j);
	}
}

static void	meta_audit(const t_meta *m)
{
	h_eq_i64("meta: heap_check", heap_check(), 0);
	h_eq_u64("meta: octets = modele", a04_bytes(), m->expect);
	h_eq_u64("meta: objets = modele", a04_objects(), m->live);
}

uint64_t	a04_meta_run(uint64_t seed, uint64_t ops)
{
	t_meta		m;
	uint64_t	i;

	a04_fresh();
	memset(&m, 0, sizeof(m));
	m.rng.state = seed;
	i = 0;
	while (i < ops)
	{
		meta_step(&m);
		if (i % 997 == 996)
			meta_audit(&m);
		i++;
	}
	meta_audit(&m);
	i = 0;
	while (i < A04_META_SLOTS)
	{
		if (m.p[i])
			meta_free(&m, (uint32_t)i);
		i++;
	}
	h_eq_u64("meta: modele vide a la fin", m.expect + m.live, 0);
	a04_drain("meta: fin de sequence");
	return (m.sum);
}
