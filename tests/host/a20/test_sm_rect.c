#include <stdint.h>
#include "velum/libk.h"
#include "harness.h"
#include "a20_test.h"
#include "layout.h"
#include "startmenu.h"

static t_smlayout	sample(int32_t ntiles)
{
	t_smlayout	l;

	memset(&l, 0, sizeof(l));
	l.panel = lay_rect(0, 0, 540, 289);
	l.tile[0] = lay_rect(24, 120, 120, 100);
	l.tile[1] = lay_rect(156, 120, 120, 100);
	l.tile[2] = lay_rect(288, 120, 120, 100);
	l.tile[3] = lay_rect(420, 120, 120, 100);
	l.logoff = lay_rect(380, 240, 40, 32);
	l.poweroff = lay_rect(430, 240, 32, 32);
	l.ntiles = ntiles;
	return (l);
}

static void	same(const char *what, t_rect a, t_rect b)
{
	h_true(a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h, what);
}

static void	layout_table_de_decision(void)
{
	t_smlayout	l;
	int32_t		n;
	int32_t		idx;
	t_rect		want;

	n = 0;
	while (n <= LUNA_LAUNCHER_TILES)
	{
		l = sample(n);
		idx = 0;
		while (idx < SM_ITEMS)
		{
			want = lay_rect(0, 0, 0, 0);
			if (idx < n)
				want = l.tile[idx];
			else if (idx == SM_IDX_LOGOFF)
				want = l.logoff;
			else if (idx == SM_IDX_SHUTDOWN)
				want = l.poweroff;
			same("rectangle de l'entree", sm_item_rect(&l, idx), want);
			idx++;
		}
		n++;
	}
}

static void	layout_compte_de_tuiles_corrompu(void)
{
	static const int32_t	counts[] = {LUNA_LAUNCHER_TILES + 1, SM_ITEMS,
		INT32_MAX};
	t_smlayout				l;
	size_t					i;

	i = 0;
	while (i < sizeof(counts) / sizeof(counts[0]))
	{
		l = sample(counts[i]);
		same("pied: fermer la session", sm_item_rect(&l, SM_IDX_LOGOFF),
			l.logoff);
		same("pied: eteindre", sm_item_rect(&l, SM_IDX_SHUTDOWN), l.poweroff);
		same("derniere tuile", sm_item_rect(&l, LUNA_LAUNCHER_TILES - 1),
			l.tile[LUNA_LAUNCHER_TILES - 1]);
		i++;
	}
}

int	main(void)
{
	h_begin("a20/startmenu-rect");
	h_run("lanceur: rectangle par entree", layout_table_de_decision);
	h_run("lanceur: compte de tuiles corrompu",
		layout_compte_de_tuiles_corrompu);
	return (h_end());
}
