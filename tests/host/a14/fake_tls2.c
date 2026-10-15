#include "fake_int.h"
#include "fake_sys.h"

static int64_t	g_next_tid = 1000;

int64_t	fake_set_fsbase(const uint64_t *a)
{
	fake_slot()->override = (t_vtcb *)(uintptr_t)a[0];
	return (0);
}

int64_t	fake_gettid(const uint64_t *a)
{
	t_fslot	*s;

	(void)a;
	s = fake_slot();
	if (!s->tid)
		s->tid = __atomic_fetch_add(&g_next_tid, 1, __ATOMIC_SEQ_CST);
	return (s->tid);
}
