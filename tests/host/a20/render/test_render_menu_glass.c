#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "render.h"

#define MENU_X 12
#define MENU_W 540
#define MENU_H 289
#define BAR_H 80

const char	*__asan_default_options(void)
{
	return ("allocator_may_return_null=1");
}

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	menu_fond_copie_du_fond_d_ecran(void)
{
	t_shell	sh;
	t_rect	at;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	at = fwm_find("Menu Démarrer")->rect;
	drv_click(&sh, sh.bar.id, rect_center(sh.reg.start));
	h_eq_u64("coin: pixel du fond d'ecran",
		gfx_get(&sh.menu.surface, (t_point){0, 0}),
		gfx_get(&sh.wall, (t_point){at.x, at.y}));
	sh_shutdown(&sh);
}

static void	menu_sans_tampon_de_verre(void)
{
	t_shell		sh;
	uint32_t	presents;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	h_true(sh_glass_alloc(&sh, (size_t)1 << 60) == E_NOMEM,
		"allocation du tampon refusee");
	h_true(sh.glass_px == NULL, "plus de tampon");
	presents = fwm_find("Menu Démarrer")->presents;
	drv_click(&sh, sh.bar.id, rect_center(sh.reg.start));
	h_eq_i64("menu ouvert malgre tout", sh.sm.open, 1);
	h_true(fwm_find("Menu Démarrer")->presents > presents, "peint en opaque");
	h_eq_u64("menu opaque au centre",
		gfx_get(&sh.menu.surface, rect_center(sh.sml.panel)) >> 24, 0xff);
	drv_click(&sh, sh.menu.id, rect_center(sh.sml.tile[1]));
	h_eq_u64("le lancement marche", g_fos.nspawn, 1);
	sh_shutdown(&sh);
}

static void	menu_suit_le_theme(void)
{
	t_shell		sh;
	t_lunatheme	t;
	t_lunatheme	other;
	uint32_t	presents;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	h_eq_i64("rien a faire", sh_theme_sync(&sh), 0);
	drv_click(&sh, sh.bar.id, rect_center(sh.reg.start));
	presents = fwm_find("Menu Démarrer")->presents;
	t = luna_theme_get();
	other = LUNA_THEME_DARK;
	if (t == LUNA_THEME_DARK)
		other = LUNA_THEME_LIGHT;
	luna_theme_set(other);
	h_eq_i64("bureau repeint", sh_theme_sync(&sh), 1);
	h_true(fwm_find("Menu Démarrer")->presents > presents, "menu repeint");
	h_eq_i64("deuxieme appel sans effet", sh_theme_sync(&sh), 0);
	luna_theme_set(t);
	h_eq_i64("retour au theme", sh_theme_sync(&sh), 1);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-menu-glass");
	h_run("rendu menu: fond copie du fond d'ecran",
		menu_fond_copie_du_fond_d_ecran);
	h_run("rendu menu: sans tampon de verre", menu_sans_tampon_de_verre);
	h_run("rendu menu: suit le theme", menu_suit_le_theme);
	return (h_end());
}
