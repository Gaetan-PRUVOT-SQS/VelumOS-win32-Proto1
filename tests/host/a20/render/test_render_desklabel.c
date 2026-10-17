#include <stdlib.h>
#include "harness.h"
#include "velum/libk.h"
#include "render.h"

static const char		*g_argv[] = {"shell", "Utilisateur", NULL};
static const int32_t	g_width[3] = {80, 70, 62};

static int32_t	delta(uint32_t a, uint32_t b)
{
	int32_t	d;
	int32_t	s;

	d = 0;
	s = 0;
	while (s < 24)
	{
		d += abs((int)((a >> s) & 0xffu) - (int)((b >> s) & 0xffu));
		s += 8;
	}
	return (d);
}

static int32_t	plate_x(const t_shell *sh, t_rect cell, int32_t from, int32_t step)
{
	int32_t	x;
	int32_t	y;

	y = cell.y + 72;
	x = from;
	while (x >= cell.x && x < cell.x + cell.w)
	{
		if (delta(gfx_get(&sh->desk.surface, (t_point){x, y}),
				sh->wall_px[y * sh->wall.w + x]) > 24)
			return (x);
		x += step;
	}
	return (from);
}

static void	etiquettes_largeur_maquette(void)
{
	t_shell	sh;
	t_rect	c;
	int32_t	w;
	int32_t	i;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	render_shot("bureau_clair");
	i = 0;
	while (i < 3)
	{
		c = sh.cells[i];
		w = plate_x(&sh, c, c.x + c.w - 1, -1) - plate_x(&sh, c, c.x, 1) + 1;
		h_true(abs(w - g_width[i]) <= 3,
			"largeur de l'etiquette: texte 12,5 / 600 + 2 x 8");
		i++;
	}
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-desklabel");
	h_run("bureau: largeur des etiquettes, mesures maquette",
		etiquettes_largeur_maquette);
	return (h_end());
}
