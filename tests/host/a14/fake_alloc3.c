#include "fake_alloc.h"
#include "stdlib.h"

static size_t	fa_pick_size(uint64_t *rng)
{
	uint64_t	r;

	r = fa_rng(rng);
	if ((r >> 40) % 16 == 0)
		return (1 + r % 20000);
	return (1 + r % 256);
}

static void	fa_job_resize(t_fa_job *j, t_fa_slot *s)
{
	size_t	n;
	void	*np;

	n = fa_pick_size(&j->seed);
	np = realloc(s->p, n);
	if (!np)
	{
		j->errors++;
		return ;
	}
	if (n > s->n)
		n = s->n;
	if (!fa_intact(np, n, s->seed))
		j->errors++;
	s->p = np;
	s->n = n;
	s->seed = (uint8_t)fa_rng(&j->seed);
	fa_fill(s->p, s->n, s->seed);
}

static void	fa_job_release(t_fa_job *j, t_fa_slot *s)
{
	if (!fa_intact(s->p, s->n, s->seed))
		j->errors++;
	free(s->p);
	s->p = NULL;
}

static void	fa_job_acquire(t_fa_job *j, t_fa_slot *s)
{
	s->n = fa_pick_size(&j->seed);
	s->seed = (uint8_t)fa_rng(&j->seed);
	s->p = malloc(s->n);
	if (!s->p)
		j->errors++;
	else
		fa_fill(s->p, s->n, s->seed);
}

void	fa_job_step(t_fa_job *j)
{
	t_fa_slot	*s;
	uint64_t	r;

	r = fa_rng(&j->seed);
	s = &j->slots[r % FA_SLOTS];
	if (s->p && (r >> 20) % 4 == 0)
		fa_job_resize(j, s);
	else if (s->p)
		fa_job_release(j, s);
	else
		fa_job_acquire(j, s);
}
