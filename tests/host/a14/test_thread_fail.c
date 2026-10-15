#include <stdint.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static uint64_t	live_blocks(void)
{
	t_vheapstats	st;

	v_heap_stats(&st);
	return (st.live_blocks);
}

static void	fail_no_memory(void)
{
	t_vthread	*t;

	fake_reset();
	fake_kernel_on();
	g_fsys.valloc_budget = 0;
	h_eq_i64("tas epuise", v_thread_create(&t, fs_ret_plus1, NULL), E_NOMEM);
	h_eq_u64("aucun fil lance", g_fsys.threads_alive, 0);
	g_fsys.valloc_budget = -1;
}

static void	fail_event_creation(void)
{
	t_vthread	*t;
	int64_t		h[256];
	uint64_t	before;
	int			n;

	fake_reset();
	fake_kernel_on();
	n = 0;
	h[0] = v_event_create(0, 0);
	while (h[n] > 0 && n < 255)
		h[++n] = v_event_create(0, 0);
	before = live_blocks();
	h_eq_i64("evenements satures", v_thread_create(&t, fs_ret_plus1, NULL),
		E_NFILE);
	h_eq_u64("descripteur libere", live_blocks(), before);
	while (n--)
		v_close((t_handle)h[n]);
}

static void	fail_kernel_create(void)
{
	t_vthread	*t;
	uint64_t	before;

	fake_reset();
	fake_kernel_on();
	before = live_blocks();
	g_fsys.thread_fail = E_AGAIN;
	h_eq_i64("noyau refuse", v_thread_create(&t, fs_ret_plus1, NULL), E_AGAIN);
	h_eq_u64("descripteur libere", live_blocks(), before);
	h_eq_u64("evenement ferme", g_fsys.events_live, 0);
}

int	main(void)
{
	h_begin("a14/thread_fail");
	h_run("thread/injection : plus de memoire", fail_no_memory);
	h_run("thread/injection : plus d'evenement", fail_event_creation);
	h_run("thread/injection : refus du noyau", fail_kernel_create);
	return (h_end());
}
