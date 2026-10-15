#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static void	proc_spawn_kill_info(void)
{
	const char	*path = "/bin/x";
	const char	args[4] = {'a', 0, 'b', 0};
	t_procinfo	info;

	fake_reset();
	g_fsys.ret = 77;
	h_eq_i64("spawn retour", v_spawn(path, args, 4, PF_SPAWN), 77);
	sys_is(SYS_PROC_SPAWN, sys_u(path), 6, sys_u(args));
	sys_is_hi(4, PF_SPAWN, 0);
	h_eq_i64("kill retour", v_proc_kill(9, -3), 77);
	sys_is(SYS_PROC_KILL, 9, (uint64_t)(int64_t)-3, 0);
	h_eq_i64("info retour", v_proc_info(0, &info), 77);
	sys_is(SYS_PROC_INFO, 0, sys_u(&info), sizeof(info));
}

static void	proc_threads(void)
{
	t_vthreadreq	rq;

	fake_reset();
	rq.entry = 0x1111;
	rq.arg = 0x2222;
	rq.stack_size = 65536;
	rq.prio = 8;
	rq.flags = 0;
	g_fsys.ret = 5;
	h_eq_i64("thread create retour", v_thread_create_raw(&rq), 5);
	sys_is(SYS_THREAD_CREATE, 0x1111, 0x2222, 65536);
	sys_is_hi(8, 0, 0);
	h_eq_i64("thread create NULL", v_thread_create_raw(NULL), E_FAULT);
	h_eq_i64("thread prio", v_thread_prio(4, 13), 5);
	sys_is(SYS_THREAD_PRIO, 4, 13, 0);
	h_eq_i64("gettid", v_gettid(), 5);
	sys_is(SYS_GETTID, 0, 0, 0);
	h_eq_i64("set_fsbase", v_set_fsbase(0xabc000), 5);
	sys_is(SYS_SET_FSBASE, 0xabc000, 0, 0);
}

static void	proc_list_exit(void)
{
	t_procinfo	list[2];

	fake_reset();
	g_fsys.ret = 2;
	h_eq_i64("proc_list", v_proc_list(list, 2), 2);
	sys_is(SYS_PROC_LIST, sys_u(list), 2, 0);
}

int	main(void)
{
	h_begin("a14/sys_proc");
	h_run("spawn, kill, info", proc_spawn_kill_info);
	h_run("fils bruts, priorite, tid, fsbase", proc_threads);
	h_run("liste", proc_list_exit);
	return (h_end());
}
