#include "fake.h"

static t_panne	g_p;

static void	setup_sc(t_pm *pm)
{
	fake_setup(pm);
	if (g_p.sc >= 1)
		h_eq_i64("depart a", fake_install(pm, "com.a", 1, 0xaa), 0);
	if (g_p.sc == 3)
		h_eq_i64("depart b", fake_install(pm, "com.b", 1, 0xbb), 0);
	g_fs.ops = 0;
}

static int	run_sc(t_pm *pm)
{
	if (g_p.sc == 0)
		return (fake_install(pm, "com.a", 1, 0xaa));
	if (g_p.sc == 1)
		return (fake_install(pm, "com.a", 2, 0xaa));
	if (g_p.sc == 2)
		return (fake_install(pm, "com.b", 1, 0xbb));
	return (pm_remove(pm, "com.a"));
}

static void	check_point(int n, int crash)
{
	t_pm	pm;
	int		r;

	setup_sc(&pm);
	g_fs.fail_at = n;
	g_fs.crash = crash;
	r = run_sc(&pm);
	g_fs.fail_at = 0;
	fake_reg_text(g_p.now);
	h_true(strcmp(g_p.now, g_p.old) == 0 || strcmp(g_p.now, g_p.neu) == 0,
		"registre ancien ou nouveau, jamais un melange");
	h_true(r < 0 || strcmp(g_p.now, g_p.neu) == 0, "succes donc nouveau");
	h_true(fake_listed_ok(&pm) >= 0, "aucun paquet liste sans fichier");
	r = run_sc(&pm);
	fake_reg_text(g_p.now);
	h_true(strcmp(g_p.now, g_p.neu) == 0, "reprise apres la panne");
	h_true(fake_listed_ok(&pm) >= 0, "liste saine apres reprise");
	h_true(!fake_fs_get("/data/apps/paquets.tmp"), "pas de provisoire");
}

static void	sweep(void)
{
	t_pm	pm;
	int		n;

	setup_sc(&pm);
	fake_reg_text(g_p.old);
	h_true(run_sc(&pm) >= 0, "scenario sans panne");
	g_p.ops = g_fs.ops;
	fake_reg_text(g_p.neu);
	h_true(strcmp(g_p.old, g_p.neu) != 0, "le scenario change le registre");
	h_true(g_p.ops >= 4, "plusieurs points de panne");
	n = 1;
	while (n <= g_p.ops)
	{
		check_point(n, 0);
		check_point(n, 1);
		n++;
	}
}

int	main(void)
{
	h_begin("d10 pannes de stockage");
	g_p.sc = 0;
	h_run("panne_chaque_operation_installation_neuve", sweep);
	g_p.sc = 1;
	h_run("panne_chaque_operation_mise_a_jour", sweep);
	g_p.sc = 2;
	h_run("panne_chaque_operation_second_paquet", sweep);
	g_p.sc = 3;
	h_run("panne_chaque_operation_retrait", sweep);
	return (h_end());
}
