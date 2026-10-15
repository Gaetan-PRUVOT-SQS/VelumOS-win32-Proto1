#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "velum/luna_dock.h"
#include "uitext.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static int32_t	widest_date(void)
{
	char	one[2];
	int32_t	best;
	int32_t	w;
	int32_t	d;

	best = 0;
	d = 0;
	while (d <= 9)
	{
		one[0] = (char)('0' + d);
		one[1] = '\0';
		w = ui_text_width(FONT_INS_11, one);
		if (w > best)
			best = w;
		d++;
	}
	return (8 * best + 2 * ui_text_width(FONT_INS_11, "/"));
}

static void	zone_notification_maquette(void)
{
	t_shell		sh;
	t_traygeo	g;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	luna_tray_geo(sh.reg.tray, &g);
	h_eq_i64("FR: marge gauche 14", g.lang.x - sh.reg.tray.x, 14);
	h_eq_i64("FR: largeur de la police 12/600", g.lang.w, 17);
	h_eq_i64("temoins: x = FR + 14", g.dots.x, g.lang.x + 17 + 14);
	h_eq_i64("temoins: 7 + 5 + 7 + 5 + 14", g.dots.w, 38);
	h_eq_i64("horloge: x = temoins + 14", g.clock.x, g.dots.x + 38 + 14);
	h_eq_i64("horloge: date la plus large 8 chiffres + 2 barres", g.clock.w,
		74);
	h_eq_i64("pastille: x = horloge + 14", g.badge.x, g.clock.x + 74 + 14);
	h_eq_i64("pastille: 30", g.badge.w, 30);
	h_eq_i64("pastille: marge droite 6", sh.reg.tray.x + sh.reg.tray.w,
		g.badge.x + 30 + 6);
	h_true(widest_date() <= g.clock.w, "date 11 px la plus large tient");
	h_true(ui_text_width(FONT_INS_BOLD_13, "00:00") <= g.clock.w,
		"heure 13 gras tient");
	h_true(ui_text_width(FONT_INS_SEMI_12, "FR") <= g.lang.w, "FR tient");
	sh_shutdown(&sh);
}

static int	ink_pixels(const t_surface *s, t_rect r)
{
	int32_t	x;
	int32_t	y;
	int		n;

	n = 0;
	y = r.y;
	while (y < r.y + r.h)
	{
		x = r.x;
		while (x < r.x + r.w)
		{
			n += (gfx_get(s, (t_point){x, y}) & 0xff) < 0x90;
			x++;
		}
		y++;
	}
	return (n);
}

static void	zone_pixels_clair_et_sombre(void)
{
	t_shell		sh;
	t_traygeo	g;
	t_point		dot1;
	t_point		dot2;
	t_point		bat;

	render_reset(A20_FIXTURE);
	luna_theme_set(LUNA_THEME_LIGHT);
	sh_boot(&sh, 2, (char **)g_argv);
	luna_tray_geo(sh.reg.tray, &g);
	dot1 = (t_point){g.dots.x + 3, sh.reg.tray.y + 20};
	dot2 = (t_point){g.dots.x + 15, sh.reg.tray.y + 20};
	h_true(px_near(&sh.bar.surface, dot1, 0x4a5163, 0), "temoin 1 clair");
	h_true(px_near(&sh.bar.surface, dot2, 0x4a5163, 0), "temoin 2 clair");
	bat = (t_point){g.dots.x + 31, sh.reg.tray.y + 20};
	h_true(!px_near(&sh.bar.surface, bat, 0x0f7d83, 40),
		"batterie sans remplissage");
	g.clock.y = sh.reg.tray.y + 22;
	g.clock.h = 16;
	h_true(ink_pixels(&sh.bar.surface, g.clock) > 20, "date sous l'heure");
	luna_theme_set(LUNA_THEME_DARK);
	sh_bar_paint(&sh);
	h_true(px_near(&sh.bar.surface, dot1, 0xaab2c3, 0), "temoin 1 sombre");
	luna_theme_set(LUNA_THEME_LIGHT);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-tray");
	h_run("zone de notification: geometrie de la maquette",
		zone_notification_maquette);
	h_run("zone de notification: pixels clair et sombre",
		zone_pixels_clair_et_sombre);
	return (h_end());
}
