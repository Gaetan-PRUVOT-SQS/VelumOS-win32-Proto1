#include <stdint.h>
#include "harness.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "velum/luna_launcher.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

const char	*__asan_default_options(void)
{
	return ("allocator_may_return_null=1");
}

static void	dock_tampon_de_verre_et_repli(void)
{
	t_shell	sh;
	t_point	mid;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	mid = (t_point){500, 40};
	h_true(sh.glass_px != NULL, "tampon de verre alloue");
	h_true(sh.glass_n >= luna_glass_need(rect_make(0, 0, 540, 540)),
		"assez pour le lanceur 540x540");
	h_true(sh.glass_n >= luna_glass_need(rect_make(0, 0, SCREEN_W - 24, 56)),
		"assez pour le dock pleine largeur");
	h_eq_i64("allocation impossible", sh_glass_alloc(&sh, (size_t)1 << 60),
		E_NOMEM);
	h_true(sh.glass_px == NULL && sh.glass_n == 0, "aucun tampon apres echec");
	sh_bar_paint(&sh);
	h_true(px_near(&sh.bar.surface, mid, 0xeef0f5, 0),
		"repli opaque: panneau opaque du theme clair");
	h_eq_i64("taille nulle refusee", sh_glass_alloc(&sh, 0), E_INVAL);
	h_eq_i64("debordement refuse", sh_glass_alloc(&sh, (size_t)-1), E_INVAL);
	h_eq_i64("reallocation", sh_glass_make(&sh), 0);
	sh_bar_paint(&sh);
	h_true(!px_near(&sh.bar.surface, mid, 0xeef0f5, 0), "verre de retour");
	sh_shutdown(&sh);
	h_true(sh.glass_px == NULL, "tampon libere a l'arret");
}

static void	feed(t_shell *sh, uint32_t type, const t_infospec *i)
{
	t_uimsg	m;

	msg_info(&m, type, i);
	sh_dispatch(sh, &m);
}

static void	dock_indicateur_de_tache_active(void)
{
	t_shell		sh;
	t_infospec	a;
	t_infospec	b;
	t_point		c;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	a = (t_infospec){10, WS_DEFAULT, WSTATE_NORMAL, 0, "Bloc-notes"};
	b = (t_infospec){11, WS_DEFAULT, WSTATE_NORMAL, 1, "Bonjour"};
	feed(&sh, WMS_WIN_ADD, &a);
	feed(&sh, WMS_WIN_ADD, &b);
	c = (t_point){sh.btn[1].x + (sh.btn[1].w - 14) / 2,
		sh.btn[1].y + sh.btn[1].h - 5};
	h_true(px_near(&sh.bar.surface, c, 0x0f7d83, 8), "indicateur: 1er pixel");
	c.x += 13;
	h_true(px_near(&sh.bar.surface, c, 0x0f7d83, 8), "indicateur: 14e pixel");
	c.x += 1;
	h_true(!px_near(&sh.bar.surface, c, 0x0f7d83, 8), "indicateur: 15 trop");
	c.x = sh.btn[0].x + (sh.btn[0].w - 14) / 2;
	h_true(!px_near(&sh.bar.surface, c, 0x0f7d83, 8), "tache inactive: aucun");
	h_eq_i64("tache: hauteur 40", sh.btn[1].h, 40);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-dock");
	h_run("rendu dock: tampon de verre et repli",
		dock_tampon_de_verre_et_repli);
	h_run("rendu dock: indicateur de tache active",
		dock_indicateur_de_tache_active);
	return (h_end());
}
