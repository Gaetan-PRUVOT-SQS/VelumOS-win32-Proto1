#include "harness.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static uint32_t	rgb_at(const t_surface *s, int32_t x, int32_t y)
{
	return (gfx_get(s, (t_point){x, y}) & 0xffffff);
}

static uint32_t	wall_at(const t_shell *sh, int32_t x, int32_t y)
{
	return (sh->wall_px[(size_t)y * (size_t)sh->wall.w + (size_t)x]
		& 0xffffff);
}

static void	barre_geometrie_du_dock(void)
{
	t_shell	sh;
	t_fwin	*f;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	f = fwm_by_id(sh.bar.id);
	h_true(f != NULL, "fenetre de barre connue");
	h_eq_i64("fenetre: y = ecran - 80", f->rect.y, SCREEN_H - 80);
	h_eq_i64("fenetre: hauteur 80", f->rect.h, 80);
	h_eq_i64("fenetre: pleine largeur", f->rect.w, SCREEN_W);
	h_eq_i64("orbe: x = 12 + 8", sh.reg.start.x, 20);
	h_eq_i64("orbe: y centre dans 12..68", sh.reg.start.y, 19);
	h_eq_i64("orbe: 42", sh.reg.start.w, 42);
	h_eq_i64("notification: bord droit", sh.reg.tray.x + sh.reg.tray.w,
		SCREEN_W - 12 - 8);
	h_eq_i64("notification: largeur 14+17+14+38+14+74+14+30+6",
		sh.reg.tray.w, 221);
	h_eq_i64("taches: x = orbe + 8 + 4", sh.reg.strip.x, 74);
	h_eq_i64("taches: bord droit = notification - 8", sh.reg.strip.x
		+ sh.reg.strip.w, sh.reg.tray.x - 8);
	h_eq_i64("notification: hauteur 42", sh.reg.tray.h, 42);
	h_eq_i64("taches: hauteur 40", sh.reg.strip.h, 40);
	h_eq_i64("taches: y", sh.reg.strip.y, 20);
	sh_shutdown(&sh);
}

static void	barre_fond_et_verre(void)
{
	t_shell		sh;
	const int	top = SCREEN_H - 80;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	h_true(sh.wall_px != NULL, "fond d'ecran present");
	h_eq_u64("marge haute: fond d'ecran", rgb_at(&sh.bar.surface, 500, 5),
		wall_at(&sh, 500, top + 5));
	h_eq_u64("coin: fond d'ecran", rgb_at(&sh.bar.surface, 3, 76),
		wall_at(&sh, 3, top + 76));
	h_eq_u64("marge basse (y 68): fond d'ecran",
		rgb_at(&sh.bar.surface, 500, 70), wall_at(&sh, 500, top + 70));
	h_true(rgb_at(&sh.bar.surface, 500, 40) != wall_at(&sh, 500, top + 40),
		"dans le dock: verre teinte");
	h_true(rgb_at(&sh.bar.surface, 300, 40) != wall_at(&sh, 300, top + 40),
		"dans le dock: verre teinte, autre colonne");
	h_true(rgb_at(&sh.bar.surface, 41, 40) != wall_at(&sh, 41, top + 40),
		"orbe: disque translucide");
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-bar");
	h_run("rendu barre: geometrie du dock", barre_geometrie_du_dock);
	h_run("rendu barre: fond d'ecran et verre", barre_fond_et_verre);
	return (h_end());
}
