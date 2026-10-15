#include <stdint.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

void	__velum_thread_entry(void *arg);

static int	create_with(uint64_t stack, uint32_t prio, uint32_t flags)
{
	t_vthreadattr	a;
	t_vthread		*t;

	a.stack_size = stack;
	a.prio = prio;
	a.flags = flags;
	return (v_thread_create_ex(&t, fs_ret_plus1, NULL, &a));
}

static void	attr_boundaries(void)
{
	fake_reset();
	fake_kernel_on();
	g_fsys.thread_fail = E_AGAIN;
	h_eq_i64("pile 16383 refusee", create_with(16383, 8, 0), E_INVAL);
	h_eq_i64("pile 16384 acceptee", create_with(16384, 8, 0), E_AGAIN);
	h_eq_i64("pile 8388608 acceptee", create_with(8388608, 8, 0), E_AGAIN);
	h_eq_i64("pile 8388609 refusee", create_with(8388609, 8, 0), E_INVAL);
	h_eq_i64("priorite 31 acceptee", create_with(0, 31, 0), E_AGAIN);
	h_eq_i64("priorite 32 refusee", create_with(0, 32, 0), E_INVAL);
	h_eq_i64("pile 100 refusee", create_with(100, 0, 0), E_INVAL);
}

static void	attr_defaults(void)
{
	t_vthread	*t;

	fake_reset();
	fake_kernel_on();
	g_fsys.thread_fail = E_AGAIN;
	h_eq_i64("echec propage", v_thread_create(&t, fs_ret_plus1, NULL), E_AGAIN);
	h_eq_u64("entree = trampoline", g_fsys.thr_args[0],
		(uint64_t)(uintptr_t)__velum_thread_entry);
	h_true(g_fsys.thr_args[1] != 0, "argument = descripteur");
	h_eq_u64("pile par defaut", g_fsys.thr_args[2], V_THREAD_STACK_DEFAULT);
	h_eq_u64("priorite par defaut", g_fsys.thr_args[3], V_PRIO_NORMAL);
	h_eq_u64("drapeaux", g_fsys.thr_args[4], 0);
}

static void	attr_passed_through(void)
{
	fake_reset();
	fake_kernel_on();
	g_fsys.thread_fail = E_AGAIN;
	create_with(65536, 13, 5);
	h_eq_u64("pile transmise", g_fsys.thr_args[2], 65536);
	h_eq_u64("priorite transmise", g_fsys.thr_args[3], 13);
	h_eq_u64("drapeaux transmis", g_fsys.thr_args[4], 5);
	create_with(0, 0, 0);
	h_eq_u64("pile 0 = defaut", g_fsys.thr_args[2], V_THREAD_STACK_DEFAULT);
	h_eq_u64("priorite 0 = normale", g_fsys.thr_args[3], V_PRIO_NORMAL);
}

int	main(void)
{
	h_begin("a14/thread_attr");
	h_run("thread/limites : attributs", attr_boundaries);
	h_run("thread/partition : valeurs par defaut", attr_defaults);
	h_run("thread/partition : valeurs transmises", attr_passed_through);
	return (h_end());
}
