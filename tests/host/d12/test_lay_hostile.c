#include <string.h>
#include "harness.h"
#include "fake.h"

static t_droid	g_d;

static void	hostile_cycle(void)
{
	t_apklay	l;

	fd_reset(&g_d);
	g_d.root = fd_add(&g_d, DV_LINEAR, 0);
	fd_add(&g_d, DV_LINEAR, 1);
	fd_add(&g_d, DV_BUTTON, 2);
	g_d.views[1].kids[1] = 1;
	g_d.views[1].kids[2] = 2;
	g_d.views[1].kids[3] = 3;
	g_d.views[1].nkids = 4;
	g_d.views[0].kids[1] = 1;
	g_d.views[0].nkids = 2;
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("cycle : une seule boite", l.n, 1);
	h_true(fd_inside(&l, (t_rect){12, 12, 376, 276}), "dans la zone");
}

static void	hostile_identifiants(void)
{
	t_apklay	l;

	fd_reset(&g_d);
	g_d.root = fd_add(&g_d, DV_LINEAR, 0);
	fd_add(&g_d, DV_TEXT, 1);
	g_d.views[0].kids[1] = 0;
	g_d.views[0].kids[2] = 65;
	g_d.views[0].kids[3] = 0xffffffffu;
	g_d.views[0].kids[4] = 40;
	g_d.views[0].nkids = 0xffffffffu;
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("un seul enfant valide", l.n, 1);
	g_d.views[0].orientation = DROID_HORIZONTAL;
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("horizontal, compte hostile", l.n, 1);
	h_true(fd_inside(&l, (t_rect){12, 12, 376, 276}), "dans la zone");
	g_d.root = 0xffffffffu;
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("racine hors table", l.n, 0);
}

static void	hostile_table_pleine(void)
{
	t_apklay	l;
	uint32_t	i;

	fd_reset(&g_d);
	i = 0;
	while (i < DROID_VIEWS_MAX)
	{
		g_d.views[i].kind = DV_LINEAR;
		g_d.views[i].orientation = i % 2;
		g_d.views[i].nkids = DROID_KIDS_MAX;
		memset(g_d.views[i].kids, 0, sizeof(g_d.views[i].kids));
		g_d.views[i].kids[0] = (i + 1) % DROID_VIEWS_MAX + 1;
		g_d.views[i].kids[1] = (i * 7) % DROID_VIEWS_MAX + 1;
		g_d.views[i].kids[15] = i + 1;
		i++;
	}
	g_d.views[63].kind = DV_BUTTON;
	g_d.root = 1;
	apklay_run(&g_d, (t_rect){0, 0, 16384, 16384}, &l);
	h_true(l.n <= 1, "graphe complet : au plus une boite");
	h_true(fd_inside(&l, (t_rect){12, 12, 16360, 16360}), "dans la zone");
}

static void	hostile_zones(void)
{
	t_apklay	l;

	fd_reset(&g_d);
	g_d.root = fd_add(&g_d, DV_BUTTON, 0);
	apklay_run(&g_d, (t_rect){0, 0, 0x7fffffff, 0x7fffffff}, &l);
	h_eq_u64("zone geante refusee", l.n, 0);
	apklay_run(&g_d, (t_rect){0x7fffffff, 0x7fffffff, 400, 300}, &l);
	h_eq_u64("origine geante refusee", l.n, 0);
	apklay_run(&g_d, (t_rect){0, 0, -5, -5}, &l);
	h_eq_u64("zone negative", l.n, 0);
	apklay_run(&g_d, (t_rect){-100, -100, 400, 300}, &l);
	h_eq_u64("origine negative acceptee", l.n, 1);
	h_eq_i64("x", l.box[0].r.x, -88);
	g_d.views[0].kind = 99;
	apklay_run(&g_d, (t_rect){0, 0, 400, 300}, &l);
	h_eq_u64("genre inconnu", l.n, 0);
}

int	main(void)
{
	h_begin("d12 mise en page hostile");
	h_run("hostile_cycle", hostile_cycle);
	h_run("hostile_identifiants", hostile_identifiants);
	h_run("hostile_table_pleine", hostile_table_pleine);
	h_run("hostile_zones", hostile_zones);
	return (h_end());
}
