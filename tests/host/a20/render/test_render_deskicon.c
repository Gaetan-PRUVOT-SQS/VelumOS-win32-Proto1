#include <stdlib.h>
#include "harness.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};
static const int32_t	g_top[3] = {6, 6, 7};
static const int32_t	g_left[3] = {22, 26, 19};

static int	changed(const t_shell *sh, int32_t x, int32_t y)
{
	uint32_t	a;
	uint32_t	b;
	int32_t		d;
	int32_t		s;

	a = gfx_get(&sh->desk.surface, (t_point){x, y});
	b = sh->wall_px[y * sh->wall.w + x];
	d = 0;
	s = 0;
	while (s < 24)
	{
		d += abs((int)((a >> s) & 0xffu) - (int)((b >> s) & 0xffu));
		s += 8;
	}
	return (d > 70);
}

static int32_t	first_row(const t_shell *sh, t_rect r)
{
	int32_t	x;
	int32_t	y;

	y = r.y;
	while (y < r.y + r.h)
	{
		x = r.x;
		while (x < r.x + r.w)
		{
			if (changed(sh, x, y))
				return (y);
			x++;
		}
		y++;
	}
	return (-1);
}

static int32_t	first_col(const t_shell *sh, t_rect r)
{
	int32_t	x;
	int32_t	y;

	x = r.x;
	while (x < r.x + r.w)
	{
		y = r.y;
		while (y < r.y + r.h)
		{
			if (changed(sh, x, y))
				return (x);
			y++;
		}
		x++;
	}
	return (-1);
}

static void	icone_au_bord_haut_de_la_cellule(void)
{
	t_shell	sh;
	t_rect	z;
	int32_t	top;
	int32_t	left;
	int32_t	i;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	i = 0;
	while (i < 3)
	{
		z = rect_make(sh.cells[i].x + 8, sh.cells[i].y, 74, 62);
		top = first_row(&sh, z) - sh.cells[i].y;
		left = first_col(&sh, z) - sh.cells[i].x;
		h_true(abs(top - g_top[i]) <= 1, "haut de l'icone: cellule + 6");
		h_true(abs(left - g_left[i]) <= 1, "bord gauche de l'icone");
		i++;
	}
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-deskicon");
	h_run("bureau: icone collee au haut de la cellule, mesures maquette",
		icone_au_bord_haut_de_la_cellule);
	return (h_end());
}
