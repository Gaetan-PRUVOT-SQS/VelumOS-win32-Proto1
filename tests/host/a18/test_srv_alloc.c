#include "harness.h"
#include "help.h"
#include "luna_int.h"

static t_wsrv	g_s;

static void	propriete_des_allocations(void)
{
	uint32_t	*a;
	uint32_t	*b;
	size_t		px;

	h_eq_i64("ouverture", hs_start(&g_s, 640, 480), 0);
	h_eq_u64("deux allocations vivantes", g_fkmem.live, 2);
	h_true(ha_has_size(640ull * 480 * 4), "fond : largeur * hauteur * 4");
	h_true(ha_has_size(ha_glass_bytes(640)), "verre : 2 * 160 * 20 * 4");
	px = (size_t)(ha_glass_bytes(640) / 8);
	h_true(lp_gscratch(px, &a, &b) && a == g_s.glass && b == a + px,
		"tampon inscrit dans luna, deux moities");
	h_true(!lp_gscratch(px + 1, &a, &b), "au dela de la moitie : refus");
	hs_stop(&g_s);
	h_true(!lp_gscratch(1, &a, &b), "tampon desinscrit apres fermeture");
	h_eq_u64("plus rien de vivant", g_fkmem.live, 0);
	h_eq_u64("aucune liberation anormale", g_fkmem.bad, 0);
}

static void	double_fermeture(void)
{
	hs_start(&g_s, 640, 480);
	ws_close_all(&g_s);
	h_eq_u64("apres la premiere fermeture", g_fkmem.live, 0);
	ws_close_all(&g_s);
	hs_stop(&g_s);
	h_eq_u64("aucune double liberation", g_fkmem.bad, 0);
	h_true(g_s.glass == NULL && g_s.back.px == NULL, "pointeurs a NULL");
	h_true(g_s.glass_bytes == 0, "taille du verre remise a zero");
}

static void	port_deja_pris(void)
{
	fk_alloc_reset();
	fk_reset();
	fk_set_display(640, 480);
	h_true(ws_sys_listen(WM_PORT_NAME) > 0, "port pris d'avance");
	h_eq_i64("ouverture refusee", ws_open(&g_s), E_EXIST);
	h_eq_u64("fond et verre liberes", g_fkmem.live, 0);
	h_eq_u64("aucune liberation anormale", g_fkmem.bad, 0);
	h_true(g_s.glass == NULL && g_s.back.px == NULL, "pointeurs a NULL");
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_alloc");
	h_run("propriete : fond et verre, tailles, inscription luna",
		propriete_des_allocations);
	h_run("fermeture repetee sans double liberation", double_fermeture);
	h_run("echec du port apres l'ecran : tout libere", port_deja_pris);
	return (h_end());
}
