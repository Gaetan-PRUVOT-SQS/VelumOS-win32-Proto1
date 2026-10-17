#include "harness.h"
#include "velum/luna_launcher.h"
#include "luna_int.h"

static const int32_t	g_in[6] = {-3, 0, 1, 4, 5, 99};
static const int32_t	g_want[6] = {0, 0, 1, 4, 4, 4};

static void	tile_count(void)
{
	t_launcherlayout	l;
	int32_t				i;

	i = 0;
	while (i < 6)
	{
		luna_launcher_layout(lp_rect(12, 431, 540, 289), g_in[i], &l);
		h_eq_i64("tuiles reelles, 4 au plus", l.ntiles, g_want[i]);
		h_eq_i64("pas de tuile fantome", l.tile[3].w > 0, g_want[i] == 4);
		i++;
	}
}

static void	hit_table(void)
{
	t_launcherlayout	l;

	luna_launcher_layout(lp_rect(12, 431, 540, 289), 2, &l);
	h_eq_i64("tuile 1", luna_launcher_hit(&l, 217, 588), 1);
	h_eq_i64("bord gauche tuile 0", luna_launcher_hit(&l, 30, 541), 0);
	h_eq_i64("juste avant la tuile 0", luna_launcher_hit(&l, 29, 541),
		LUNA_LAUNCHER_NONE);
	h_eq_i64("bord droit tuile 0", luna_launcher_hit(&l, 150, 636), 0);
	h_eq_i64("espacement entre tuiles", luna_launcher_hit(&l, 151, 588),
		LUNA_LAUNCHER_NONE);
	h_eq_i64("dernier pixel de la tuile 0", luna_launcher_hit(&l, 100, 636),
		0);
	h_eq_i64("sous la tuile 0", luna_launcher_hit(&l, 100, 637),
		LUNA_LAUNCHER_NONE);
	h_eq_i64("emplacement de tuile absente", luna_launcher_hit(&l, 471, 588),
		LUNA_LAUNCHER_NONE);
	h_eq_i64("fermer la session", luna_launcher_hit(&l, 417, 684),
		LUNA_LAUNCHER_LOGOFF);
	h_eq_i64("eteindre", luna_launcher_hit(&l, 516, 684),
		LUNA_LAUNCHER_POWER);
	h_eq_i64("coin du panneau", luna_launcher_hit(&l, 12, 431),
		LUNA_LAUNCHER_NONE);
	h_eq_i64("disposition nulle", luna_launcher_hit(NULL, 0, 0),
		LUNA_LAUNCHER_NONE);
}

int	main(void)
{
	h_begin("a17/lanceur_clic");
	h_run("partition du nombre de tuiles, limites", tile_count);
	h_run("table de clic, bords inclus et exclus", hit_table);
	return (h_end());
}
