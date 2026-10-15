#include "obj_int.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/pmm.h"

static uint64_t	st_heap_live(void)
{
	t_heap_stats	hs;

	heap_get_stats(&hs);
	return (hs.allocs_live[HEAP_OBJECT] + hs.allocs_live[HEAP_IPC]);
}

uint64_t	st_pmm_user(void)
{
	t_pmm_stats	ps;

	pmm_get_stats(&ps);
	return (ps.owned[PMM_USER]);
}

static int	st_run(const char *name, int (*fn)(void))
{
	uint64_t	before;
	int			rc;

	before = st_heap_live();
	rc = fn();
	if (rc == 0 && st_heap_live() != before)
		rc = E_IO;
	if (rc != 0)
		klog_err("objets: autotest %s en echec (%d)", name, rc);
	return (rc != 0);
}

int	st_handles(void)
{
	t_process	p;
	t_object	*o;
	t_handle	h;
	t_handle	h2;
	int			bad;

	memset(&p, 0, sizeof(p));
	p.handles = handle_table_create();
	if (!p.handles)
		return (E_NOMEM);
	bad = 1;
	if (evt_create(1, 0, &o) == 0)
	{
		bad = (handle_alloc(&p, o, RIGHTS_SIGOBJ, &h) != 0);
		obj_unref(o);
		bad += (handle_dup(&p, h, &p, &h2) != 0);
		bad += (handle_close(&p, h) != 0);
		bad += (handle_close(&p, h) != E_BADF);
		bad += (handle_close(&p, h2) != 0);
	}
	handle_table_destroy(p.handles);
	return (bad);
}

int	object_selftest(void)
{
	int	fails;

	fails = st_run("handles", st_handles);
	fails += st_run("canal", st_chan);
	fails += st_run("section", st_section);
	return (fails);
}
