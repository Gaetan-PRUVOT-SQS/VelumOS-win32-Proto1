#include "errno.h"
#include "fake_check.h"
#include "fake_f1_buf.h"
#include "fake_sys.h"
#include "harness.h"
#include "string.h"
#include "velum/velum.h"

static char	g_big[3001];

static void	spawnv_args(void)
{
	const char	*path = "/system/bin/shell";
	const char	*argv[3];
	const char	*none[1];

	fake_reset();
	fake_kernel_on();
	g_fsys.ret = 321;
	f1_fill_argv(argv, 2, "shell");
	argv[1] = "-x";
	h_eq_i64("spawnv retour", v_spawnv(path, argv, PF_SPAWN | PF_INPUT), 321);
	sys_is(SYS_PROC_SPAWN, sys_u(path), 17, g_fsys.last_args[2]);
	sys_is_hi(3, PF_SPAWN | PF_INPUT, 0);
	h_true(g_fsys.last_args[2] != 0, "blob transmis");
	h_true(!(g_fsys.last_args[2] & 15), "blob aligne sur 16");
	none[0] = NULL;
	h_eq_i64("spawnv argv vide", v_spawnv("/p", none, 3), 321);
	sys_is(SYS_PROC_SPAWN, g_fsys.last_args[0], 2, g_fsys.last_args[2]);
	sys_is_hi(0, 3, 0);
}

static void	spawnv_errors(void)
{
	const char		*argv[260];
	t_vheapstats	before;
	t_vheapstats	after;

	fake_reset();
	fake_kernel_on();
	v_heap_stats(&before);
	h_eq_i64("spawnv argv NULL", v_spawnv("/x", NULL, 0), E_FAULT);
	f1_fill_argv(argv, V_ARGC_MAX + 2, "x");
	h_eq_i64("spawnv 258 entrees", v_spawnv("/x", argv, 0), E_INVAL);
	sys_none();
	f1_fill_argv(argv, 2, "x");
	h_eq_i64("spawnv chemin NULL", v_spawnv(NULL, argv, 0), E_FAULT);
	v_heap_stats(&after);
	h_eq_u64("chemin NULL : blob libere",
		after.live_blocks, before.live_blocks);
	h_eq_u64("chemin NULL : octets vivants",
		after.live_bytes, before.live_bytes);
}

static void	spawnv_nomem(void)
{
	const char		*argv[3];
	t_vheapstats	before;
	t_vheapstats	after;

	memset(g_big, 'z', 3000);
	g_big[3000] = '\0';
	f1_fill_argv(argv, 2, g_big);
	fake_reset();
	fake_kernel_on();
	g_fsys.ret = 321;
	g_fsys.valloc_budget = 0;
	v_heap_stats(&before);
	h_eq_i64("spawnv sans memoire", v_spawnv("/x", argv, 0), E_NOMEM);
	h_eq_u64("une seule tentative VALLOC", g_fsys.calls, 1);
	h_eq_u64("c'etait SYS_VALLOC", g_fsys.last_num, SYS_VALLOC);
	h_eq_i64("errno ENOMEM", errno, ENOMEM);
	v_heap_stats(&after);
	h_eq_u64("aucun bloc vivant de plus",
		after.live_blocks, before.live_blocks);
}

static void	spawnv_leak(void)
{
	const char		*argv[3];
	t_vheapstats	before;
	t_vheapstats	after;

	f1_fill_argv(argv, 2, g_big);
	fake_reset();
	fake_kernel_on();
	g_fsys.ret = 99;
	v_heap_stats(&before);
	h_eq_i64("spawnv gros blob", v_spawnv("/x", argv, 1), 99);
	v_heap_stats(&after);
	h_eq_u64("blocs vivants rendus", after.live_blocks, before.live_blocks);
	h_eq_u64("gros blocs rendus", after.large_bytes, before.large_bytes);
	h_eq_u64("une page allouee", g_fsys.valloc_calls, 1);
	h_eq_u64("une page rendue", g_fsys.vfree_calls, 1);
	h_eq_u64("aucun mappage restant", g_fsys.live_maps, 0);
	h_eq_u64("aucun VFREE invalide", g_fsys.vfree_bad, 0);
}

int	main(void)
{
	h_begin("a14/sys_spawnv");
	h_run("v_spawnv/exigence : chemin, blob, longueur, flags", spawnv_args);
	h_run("v_spawnv/supposition d'erreur : argv nul, 257, chemin nul",
		spawnv_errors);
	h_run("v_spawnv/injection de panne : VALLOC refuse", spawnv_nomem);
	h_run("v_spawnv/flot de donnees : aucun bloc ni page restant",
		spawnv_leak);
	return (h_end());
}
