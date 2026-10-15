#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static void	null_paths(void)
{
	t_vstat	st;

	fake_reset();
	g_fsys.ret = 77;
	h_eq_i64("open chemin nul", v_open(NULL, O_RDONLY, 0), E_FAULT);
	h_eq_i64("stat chemin nul", v_stat(NULL, &st), E_FAULT);
	h_eq_i64("mkdir chemin nul", v_mkdir(NULL, 0x1ed), E_FAULT);
	h_eq_i64("unlink chemin nul", v_unlink(NULL), E_FAULT);
	h_eq_i64("rename ancien nul", v_rename(NULL, "/b"), E_FAULT);
	h_eq_i64("rename nouveau nul", v_rename("/a", NULL), E_FAULT);
	h_eq_i64("rename deux nuls", v_rename(NULL, NULL), E_FAULT);
	sys_none();
}

static void	null_names(void)
{
	fake_reset();
	g_fsys.ret = 77;
	h_eq_i64("listen nom nul", v_port_listen(NULL, 0), E_FAULT);
	h_eq_i64("connect nom nul", v_port_connect(NULL, 0), E_FAULT);
	h_eq_i64("spawn chemin nul", v_spawn(NULL, NULL, 0, 0), E_FAULT);
	h_eq_i64("spawn chemin nul avec arguments",
		v_spawn(NULL, "a", 2, PF_SPAWN), E_FAULT);
	h_eq_i64("log message nul", v_log(V_LOG_ERR, NULL), E_FAULT);
	sys_none();
}

static void	null_not_checked(void)
{
	fake_reset();
	g_fsys.ret = 77;
	h_eq_i64("spawn arguments nuls transmis", v_spawn("/x", NULL, 0, 3), 77);
	sys_is(SYS_PROC_SPAWN, g_fsys.last_args[0], 2, 0);
	sys_is_hi(0, 3, 0);
	h_eq_u64("un seul appel noyau", g_fsys.calls, 1);
	h_eq_i64("stat sortie nulle transmise", v_stat("/y", NULL), 77);
	sys_is(SYS_STAT, g_fsys.last_args[0], 2, 0);
}

int	main(void)
{
	h_begin("a14/sys_nullptr");
	h_run("chemins/supposition d'erreur : pointeur nul refuse localement",
		null_paths);
	h_run("noms, spawn, log/supposition d'erreur : pointeur nul refuse",
		null_names);
	h_run("spawn, stat/partition : sortie ou arguments nuls laisses au noyau",
		null_not_checked);
	return (h_end());
}
