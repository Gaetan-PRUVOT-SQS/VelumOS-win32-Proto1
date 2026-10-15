#include <stdlib.h>
#include "harness.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static int	blend_near(uint32_t got, uint32_t bg, uint32_t ink)
{
	int32_t	s;
	int32_t	want;

	s = 0;
	while (s < 24)
	{
		want = ((int32_t)((bg >> s) & 0xffu)
				+ (int32_t)((ink >> s) & 0xffu)) / 2;
		if (abs((int32_t)((got >> s) & 0xffu) - want) > 2)
			return (0);
		s += 8;
	}
	return (1);
}

static int	glyph_row(const t_shell *sh, t_rect cell, int32_t y)
{
	uint32_t	px;
	int32_t		x;

	x = cell.x + 15;
	while (x < cell.x + 20)
	{
		px = gfx_get(&sh->desk.surface, (t_point){x, y});
		if ((px & 0xffu) + ((px >> 8) & 0xffu) + ((px >> 16) & 0xffu) < 250)
			return (1);
		x++;
	}
	return (0);
}

static void	lettre_sur_la_ligne_de_base_maquette(void)
{
	t_shell	sh;
	t_rect	c;
	int32_t	top;
	int32_t	bottom;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	c = sh.cells[0];
	top = c.y + 58;
	while (top < c.y + c.h && !glyph_row(&sh, c, top))
		top++;
	bottom = c.y + c.h - 1;
	while (bottom > top && !glyph_row(&sh, c, bottom))
		bottom--;
	h_true(abs(top - c.y - 66) <= 1, "haut du O de Ordinateur: cellule + 66");
	h_true(abs(bottom - c.y - 74) <= 1, "bas du O de Ordinateur: cellule + 74");
	sh_shutdown(&sh);
}

static void	etiquette_blanc_ou_encre_a_50_pour_cent(void)
{
	t_shell		sh;
	t_point		at;
	uint32_t	bg;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	at = (t_point){sh.cells[0].x + 7, sh.cells[0].y + 72};
	bg = sh.wall_px[at.y * sh.wall.w + at.x];
	h_true(blend_near(gfx_get(&sh.desk.surface, at), bg, 0xffffffu),
		"clair: etiquette blanc 50 %");
	luna_theme_set(LUNA_THEME_DARK);
	sh_desk_paint(&sh);
	render_shot("bureau_sombre");
	bg = sh.wall_px[at.y * sh.wall.w + at.x];
	h_true(blend_near(gfx_get(&sh.desk.surface, at), bg, 0x141824u),
		"sombre: etiquette encre 50 %");
	luna_theme_set(LUNA_THEME_LIGHT);
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-deskplate");
	h_run("bureau: fond d'etiquette blanc ou encre a 50 %",
		etiquette_blanc_ou_encre_a_50_pour_cent);
	h_run("bureau: lettre O sur les rangees de la maquette",
		lettre_sur_la_ligne_de_base_maquette);
	return (h_end());
}
