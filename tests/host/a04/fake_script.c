#include <string.h>
#include "a04_fake.h"
#include "velum/util.h"

static const uint32_t	g_sizes[8] = {16, 100, 700, 2048, 3000, 9000, 70000,
	24};

static void	script_alloc(t_script *s, uint32_t i, uint32_t j)
{
	size_t	n;
	uint8_t	*p;

	n = g_sizes[(i * 3 + j) % 8];
	if (i % 5 == 0)
		p = kmalloc(n);
	else if (i % 5 == 1)
		p = kmalloc_aligned(n, (size_t)64 << (i % 3));
	else if (i % 5 == 2)
		p = kcalloc(1, n);
	else if (i % 5 == 3)
		p = kmalloc_tag(n, (t_heap_tag)(i % HEAP_TAGS));
	else
		p = kzalloc(n);
	s->p[j] = p;
	s->n[j] = (uint32_t)n;
	if (p)
		a04_fill(p, n, (uint8_t)(j + 1));
}

static void	script_resize(t_script *s, uint32_t i, uint32_t j)
{
	uint32_t	to;
	uint8_t		*q;

	to = g_sizes[(i * 7 + j) % 8];
	q = krealloc(s->p[j], to);
	if (!q)
	{
		h_eq_i64("script: bloc intact apres echec", a04_verify(s->p[j],
				s->n[j], (uint8_t)(j + 1)), 0);
		return ;
	}
	h_eq_i64("script: prefixe conserve", a04_verify(q, min_u64(s->n[j], to),
			(uint8_t)(j + 1)), 0);
	s->p[j] = q;
	s->n[j] = to;
	a04_fill(q, to, (uint8_t)(j + 1));
}

void	a04_script_run(t_script *s)
{
	uint32_t	i;
	uint32_t	j;

	memset(s, 0, sizeof(*s));
	i = 0;
	while (i < 60)
	{
		j = i % A04_SCRIPT_SLOTS;
		if (!s->p[j])
			script_alloc(s, i, j);
		else if (i % 3 == 0)
		{
			h_eq_i64("script: contenu avant liberation", a04_verify(s->p[j],
					s->n[j], (uint8_t)(j + 1)), 0);
			kfree(s->p[j]);
			s->p[j] = NULL;
		}
		else
			script_resize(s, i, j);
		i++;
	}
}

void	a04_script_end(t_script *s)
{
	uint32_t	j;

	j = 0;
	while (j < A04_SCRIPT_SLOTS)
	{
		if (s->p[j])
		{
			h_eq_i64("script: contenu a la fin", a04_verify(s->p[j], s->n[j],
					(uint8_t)(j + 1)), 0);
			kfree(s->p[j]);
			s->p[j] = NULL;
		}
		j++;
	}
}
