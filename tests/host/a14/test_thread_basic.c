#include <stdint.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "errno.h"
#include "velum/velum.h"

static void	thread_join_result(void)
{
	t_vthread	*t;
	void		*res;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	v_tcb()->tid = 1;
	h_true(v_thread_self() == NULL, "fil principal : pas de t_vthread");
	h_eq_i64("create", v_thread_create(&t, fs_ret_plus1, (void *)41), 0);
	h_eq_i64("join", v_thread_join(t, &res), 0);
	h_eq_u64("resultat transmis", (uintptr_t)res, 42);
	h_eq_i64("create (self)", v_thread_create(&t, fs_self_check, (void *)7), 0);
	v_thread_join(t, &res);
	h_eq_u64("v_thread_self() dans le fil", (uintptr_t)res, 1);
	h_true(fs_wait_threads(), "plus aucun fil vivant");
	h_eq_u64("evenements fermes", g_fsys.events_live, 0);
}

static uintptr_t	serial_sum(void)
{
	t_vthread	*t;
	void		*res;
	uintptr_t	sum;
	int			i;

	sum = 0;
	i = -1;
	while (++i < 50)
	{
		v_thread_create(&t, fs_ret_plus1, (void *)(uintptr_t)i);
		v_thread_join(t, &res);
		sum += (uintptr_t)res;
	}
	return (sum);
}

static void	thread_serial_and_parallel(void)
{
	t_vthread	*t[16];
	void		*res;
	uintptr_t	bad;
	int			i;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	h_eq_u64("50 fils en serie", serial_sum(), 50 * 49 / 2 + 50);
	i = -1;
	while (++i < 16)
		v_thread_create(&t[i], fs_ret_plus1, (void *)(uintptr_t)i);
	bad = 0;
	while (i--)
	{
		if (v_thread_join(t[i], &res) != 0)
			bad++;
		else if ((uintptr_t)res != (uintptr_t)i + 1)
			bad++;
	}
	h_eq_u64("16 fils simultanes joints", bad, 0);
	h_true(fs_wait_threads(), "plus aucun fil vivant");
}

static void	thread_errno_per_thread(void)
{
	t_vthread	*t[8];
	void		*res;
	uintptr_t	ok;
	int			i;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	errno = 77;
	i = -1;
	while (++i < 8)
		v_thread_create(&t[i], fs_errno_check, (void *)(uintptr_t)(i + 1));
	ok = 0;
	while (i--)
	{
		v_thread_join(t[i], &res);
		ok += (uintptr_t)res;
	}
	h_eq_u64("errno propre a chacun des 8 fils", ok, 8);
	h_eq_i64("errno du fil principal intact", errno, 77);
}

int	main(void)
{
	h_begin("a14/thread_basic");
	h_run("thread/etats : create et join", thread_join_result);
	h_run("thread/limite : 50 en serie, 16 en parallele",
		thread_serial_and_parallel);
	h_run("thread/partition : errno par fil", thread_errno_per_thread);
	return (h_end());
}
