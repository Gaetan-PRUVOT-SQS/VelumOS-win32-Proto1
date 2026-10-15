#include <pthread.h>
#include <stdint.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static void	states_double_join(void)
{
	t_vevent	gate;
	t_vthread	*t;
	t_fsjob		job;
	pthread_t	th;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	v_event_init(&gate, 1, 0);
	v_thread_create(&t, fs_gate_thread, &gate);
	job.counter = (uint64_t *)&t;
	pthread_create(&th, NULL, fs_join_worker, &job);
	h_true(fs_wait_waiters(t->done.handle, 1), "un join attend");
	h_eq_i64("second join concurrent", v_thread_join(t, NULL), E_INVAL);
	h_eq_i64("detach pendant un join", v_thread_detach(t), E_INVAL);
	v_event_set(&gate);
	pthread_join(th, NULL);
	h_eq_i64("premier join reussi", job.rc, 0);
	v_event_destroy(&gate);
	h_true(fs_wait_threads(), "plus aucun fil vivant");
}

static void	states_join_self(void)
{
	t_vthread	*t;
	int			rc;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	rc = 0;
	v_thread_create(&t, fs_join_self, &rc);
	v_thread_join(t, NULL);
	h_eq_i64("join de soi-meme", rc, E_DEADLK);
	h_true(fs_wait_threads(), "plus aucun fil vivant");
}

static void	states_null_arguments(void)
{
	t_vthread	*t;

	fake_reset();
	fake_kernel_on();
	h_eq_i64("join(NULL)", v_thread_join(NULL, NULL), E_INVAL);
	h_eq_i64("detach(NULL)", v_thread_detach(NULL), E_INVAL);
	h_eq_i64("create sans sortie", v_thread_create(NULL, fs_ret_plus1, NULL),
		E_INVAL);
	h_eq_i64("create sans fonction", v_thread_create(&t, NULL, NULL), E_INVAL);
	h_eq_u64("aucun appel noyau", g_fsys.calls, 0);
}

static void	states_detach_then_join(void)
{
	t_vevent	gate;
	t_vthread	*t;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	v_event_init(&gate, 1, 0);
	v_thread_create(&t, fs_gate_thread, &gate);
	h_eq_i64("detach d'un fil en cours", v_thread_detach(t), 0);
	h_eq_i64("join apres detach", v_thread_join(t, NULL), E_INVAL);
	h_eq_i64("second detach", v_thread_detach(t), E_INVAL);
	v_event_set(&gate);
	h_true(fs_wait_threads(), "le fil detache se termine");
	v_event_destroy(&gate);
}

int	main(void)
{
	h_begin("a14/thread_states");
	h_run("thread/table de decision : double join", states_double_join);
	h_run("thread/table de decision : join de soi", states_join_self);
	h_run("thread/partition : arguments nuls", states_null_arguments);
	h_run("thread/table de decision : detach puis join",
		states_detach_then_join);
	return (h_end());
}
