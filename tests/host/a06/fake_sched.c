#include <stdlib.h>
#include <string.h>
#include "fake_sched.h"

t_fakethr	*fake_self(void)
{
	t_fakethr	*f;

	f = fh_tls_get();
	if (f)
		return (f);
	f = calloc(1, sizeof(*f));
	if (!f)
		abort();
	f->kt.t.state = TS_RUNNING;
	f->kt.t.prio = PRIO_NORMAL;
	f->kt.t.base_prio = PRIO_NORMAL;
	f->iflag = RFLAGS_IF;
	fh_lock();
	f->kt.t.tid = ++g_ft.next_tid;
	f->next = g_ft.threads;
	g_ft.threads = f;
	fh_unlock();
	fh_tls_set(f);
	return (f);
}

t_kthread	*sched_kself(void)
{
	return (&fake_self()->kt);
}

t_lockstack	*sched_lockstack(void)
{
	return (&fake_self()->ls);
}

uint32_t	sched_preempt_count(void)
{
	return (fake_self()->preempt);
}

void	fake_reset_self(void)
{
	t_fakethr	*f;

	f = fake_self();
	f->preempt = 0;
	f->ls.depth = 0;
	f->iflag = RFLAGS_IF;
	f->kt.canceled = 0;
	f->kt.reason = 0;
	f->kt.token = 0;
}
