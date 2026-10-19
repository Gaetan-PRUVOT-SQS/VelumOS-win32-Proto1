#ifndef SMP_FIXTURE_H
# define SMP_FIXTURE_H

# include <stdlib.h>
# include "fake_sched.h"

typedef struct s_tcase
{
	t_kthread	kt;
	uint32_t	ran;
	int			running;
	int			overlap;
}	t_tcase;

static inline t_tcase	*tcase_new(int32_t prio, uint32_t kflags)
{
	t_tcase	*c;

	c = calloc(1, sizeof(*c));
	if (!c)
		abort();
	c->kt.t.prio = prio;
	c->kt.t.state = TS_READY;
	c->kt.kflags = kflags;
	return (c);
}

static inline void	cs_new(t_cpusched *cs, uint32_t id)
{
	cs->id = id;
	spin_init(&cs->lock, "runq");
	runq_init(&cs->rq);
}

#endif
