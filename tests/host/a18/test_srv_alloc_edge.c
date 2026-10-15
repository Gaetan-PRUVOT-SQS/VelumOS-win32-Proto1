#include "harness.h"
#include "help.h"
#include "luna_int.h"

static t_wsrv	g_s;

static void	rang(int64_t fail, int32_t status, uint32_t live)
{
	uint32_t	*a;
	uint32_t	*b;

	h_eq_i64("statut d'ouverture", ha_start(&g_s, fail, 640, 480), status);
	h_eq_u64("allocations vivantes", g_fkmem.live, live);
	h_true(lp_gscratch(1, &a, &b) == (live == 2), "verre inscrit ou repli");
	hs_stop(&g_s);
	h_eq_u64("tout libere", g_fkmem.live, 0);
	h_eq_u64("aucune liberation anormale", g_fkmem.bad, 0);
}

static void	echec_par_rang(void)
{
	rang(0, E_NOMEM, 0);
	rang(1, 0, 1);
	rang(2, 0, 2);
}

static void	cas_limite(uint32_t w, uint32_t h, int32_t status, uint32_t live)
{
	h_eq_i64("statut", ha_start(&g_s, -1, w, h), status);
	h_eq_u64("allocations vivantes", g_fkmem.live, live);
	if (live == 2)
	{
		h_true(ha_has_size((uint64_t)w * h * 4), "taille du fond");
		h_true(ha_has_size(ha_glass_bytes(w)), "taille du verre");
		ws_mark(&g_s, g_s.dirty.screen);
		ws_frame(&g_s);
		h_eq_u64("une image composee", g_s.frames, 1);
	}
	hs_stop(&g_s);
	h_eq_u64("tout libere", g_fkmem.live, 0);
	h_eq_u64("aucune liberation anormale", g_fkmem.bad, 0);
}

static void	valeurs_limites(void)
{
	cas_limite(1, 1, 0, 2);
	cas_limite(8192, 1, 0, 2);
	cas_limite(1, 8192, 0, 2);
	cas_limite(8193, 1, E_NODEV, 0);
	cas_limite(1, 8193, E_NODEV, 0);
	cas_limite(0, 480, E_NODEV, 0);
}

int	main(void)
{
	h_begin("a18/srv_alloc_edge");
	h_run("echec d'allocation, partition par rang", echec_par_rang);
	h_run("dimensions limites de l'ecran", valeurs_limites);
	return (h_end());
}
