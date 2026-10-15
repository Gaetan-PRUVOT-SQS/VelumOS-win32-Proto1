#include <stdint.h>
#include <unistd.h>
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

static void	detach_running(void)
{
	t_vevent	gate;
	t_vthread	*t;
	uint64_t	before;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	v_event_init(&gate, 1, 0);
	before = live_blocks();
	v_thread_create(&t, fs_gate_thread, &gate);
	v_thread_detach(t);
	v_event_set(&gate);
	h_true(fs_wait_threads(), "le fil detache se termine");
	h_eq_u64("descripteur libere par le fil", live_blocks(), before);
	v_event_destroy(&gate);
	h_eq_u64("evenements fermes", g_fsys.events_live, 0);
}

static void	detach_finished(void)
{
	t_vthread	*t;
	uint64_t	before;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	before = live_blocks();
	v_thread_create(&t, fs_ret_plus1, NULL);
	h_true(fs_wait_threads(), "le fil est fini");
	h_eq_i64("detach d'un fil fini", v_thread_detach(t), 0);
	h_eq_u64("descripteur libere par le detacheur", live_blocks(), before);
	h_eq_u64("evenements fermes", g_fsys.events_live, 0);
}

static void	detach_stress(void)
{
	t_vthread	*t;
	uint64_t	before;
	int			i;
	int			rc;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	before = live_blocks();
	i = 0;
	while (i < 1000)
	{
		rc = v_thread_create(&t, fs_ret_plus1, NULL);
		if (rc == E_AGAIN)
			usleep(200);
		else
		{
			v_thread_detach(t);
			i++;
		}
	}
	h_true(fs_wait_threads(), "tous les fils detaches se terminent");
	h_eq_u64("aucune fuite de descripteur", live_blocks(), before);
	h_eq_u64("evenements fermes", g_fsys.events_live, 0);
}

int	main(void)
{
	h_begin("a14/thread_detach");
	h_run("thread/etats : detach d'un fil en cours", detach_running);
	h_run("thread/etats : detach d'un fil fini", detach_finished);
	h_run("thread/aleatoire : 1000 fils detaches", detach_stress);
	return (h_end());
}
