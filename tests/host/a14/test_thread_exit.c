#include <stdint.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static void	exit_from_main_thread(void)
{
	v_thread_exit((void *)3);
}

static void	thread_exit_inside(void)
{
	t_vthread	*t;
	void		*res;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	v_thread_create(&t, fs_exit_inside, (void *)21);
	h_eq_i64("join", v_thread_join(t, &res), 0);
	h_eq_u64("resultat de v_thread_exit", (uintptr_t)res, 42);
	h_true(fs_wait_threads(), "plus aucun fil vivant");
}

static void	thread_exit_main(void)
{
	fake_reset();
	fake_kernel_on();
	h_eq_i64("v_thread_exit du fil principal = exit(code)",
		fake_exit_catch(exit_from_main_thread), 3);
}

static void	thread_tid_distinct(void)
{
	t_vthread	*t;
	int64_t		tid;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	v_tcb()->tid = 1;
	tid = 0;
	v_thread_create(&t, fs_tid_get, &tid);
	v_thread_join(t, NULL);
	h_true(tid != 0 && tid != 1, "tid du fil different de celui du principal");
	h_true(fs_wait_threads(), "plus aucun fil vivant");
}

int	main(void)
{
	h_begin("a14/thread_exit");
	h_run("thread/etats : v_thread_exit depuis un fil", thread_exit_inside);
	h_run("thread/etats : v_thread_exit depuis le principal", thread_exit_main);
	h_run("thread/partition : tid distinct", thread_tid_distinct);
	return (h_end());
}
