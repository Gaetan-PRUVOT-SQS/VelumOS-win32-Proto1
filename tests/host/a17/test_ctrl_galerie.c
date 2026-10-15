#include <stdlib.h>
#include "harness.h"
#include "lt_ctl_gal.h"
#include "velum/theme.h"

static t_lt	g_t;

static void	paint(t_color back)
{
	lt_clear(&g_t, back);
	lt_ctl_buttons(&g_t.s, 16, 14, false);
	lt_ctl_buttons(&g_t.s, 16, 58, true);
	lt_ctl_choices(&g_t.s, 16, 108);
	lt_ctl_fields(&g_t.s, 16, 178);
	lt_ctl_bars(&g_t.s, 16, 250);
	lt_ctl_menu(&g_t.s, 330, 108);
}

static void	write_png(const char *name)
{
	char		path[256];
	const char	*dir;
	int			n;

	dir = getenv("LT_OUT");
	if (dir == NULL || dir[0] == '\0')
		dir = "build/a17";
	n = snprintf(path, sizeof(path), "%s/%s", dir, name);
	h_true(n > 0 && (size_t)n < sizeof(path), "chemin de sortie");
	h_true(lt_png(path, &g_t.s, 2) == 0, "ecriture png");
}

static void	galerie_clair_et_sombre(void)
{
	h_true(lt_open(&g_t, 600, 340) == 0, "allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	paint(VT_OPAQUE_CONTENT);
	write_png("ctrl_clair.png");
	luna_theme_set(LUNA_THEME_DARK);
	paint(VT_OPAQUE_CONTENT_D);
	write_png("ctrl_sombre.png");
	h_true(lt_guard_ok(&g_t), "aucune ecriture hors de la surface");
	luna_theme_set(LUNA_THEME_LIGHT);
	lt_close(&g_t);
}

int	main(void)
{
	h_begin("a17/ctrl_galerie");
	h_run("galerie des controles, clair et sombre", galerie_clair_et_sombre);
	return (h_end());
}
