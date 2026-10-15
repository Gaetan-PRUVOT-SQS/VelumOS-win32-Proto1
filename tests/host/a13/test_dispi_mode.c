#include "cases.h"
#include "fakes.h"

static void	decision_table(void)
{
	t_dispi	d;
	int		i;

	fake_dispi_ready(&d);
	i = 0;
	while (g_mode_cases[i].name)
	{
		g_h.name = g_mode_cases[i].name;
		h_true(dispi_mode_ok(&d, g_mode_cases[i].w, g_mode_cases[i].h)
			== g_mode_cases[i].want, "admission du mode");
		i++;
	}
}

static void	vram_limits(void)
{
	t_dispi	d;

	fake_dispi_ready(&d);
	d.vram = 800ull * 600 * 4;
	h_true(dispi_mode_ok(&d, 800, 600), "800x600 tient pile");
	d.vram--;
	h_true(!dispi_mode_ok(&d, 800, 600), "un octet de moins : refus");
	d.vram = 1920ull * 1080 * 4;
	h_true(dispi_mode_ok(&d, 1920, 1080), "1920x1080 tient pile");
	d.vram--;
	h_true(!dispi_mode_ok(&d, 1920, 1080), "1920x1080 un octet de moins");
	d.vram = 0;
	h_true(!dispi_mode_ok(&d, 800, 600), "vram nulle : tout refuse");
	d.vram = 16777216;
	d.boot_w = 1366;
	d.boot_h = 768;
	h_true(dispi_mode_ok(&d, 1366, 768), "mode de demarrage hors liste admis");
	h_true(!dispi_mode_ok(&d, 1366, 769), "voisin du mode de demarrage refuse");
	d.boot_w = 0;
	d.boot_h = 0;
	h_true(!dispi_mode_ok(&d, 0, 0), "mode de demarrage nul refuse");
}

static void	list_basic(void)
{
	t_dispi		d;
	t_dispmode	m[8];
	t_modelist	l;

	fake_dispi_ready(&d);
	l.out = m;
	l.max = 8;
	l.cur_w = 1024;
	l.cur_h = 768;
	h_eq_u64("5 modes", dispi_list(&d, &l), 5);
	h_eq_u64("premier : largeur", m[0].width, 800);
	h_eq_u64("dernier : hauteur", m[4].height, 1080);
	h_eq_u64("bpp", m[2].bpp, 32);
	h_eq_u64("mode courant marque", m[1].flags, DISP_MODE_CURRENT);
	h_eq_u64("autre mode non marque", m[0].flags, 0);
	l.max = 2;
	h_eq_u64("max 2", dispi_list(&d, &l), 2);
	h_eq_u64("total rapporte", l.total, 5);
}

static void	list_variants(void)
{
	t_dispi		d;
	t_dispmode	m[8];
	t_modelist	l;

	fake_dispi_ready(&d);
	l.out = NULL;
	l.max = 0;
	l.cur_w = 1024;
	l.cur_h = 768;
	h_eq_u64("interrogation du nombre", dispi_list(&d, &l), 5);
	l.max = 4;
	h_eq_u64("sortie nulle : rien d'ecrit", dispi_list(&d, &l), 0);
	l.out = m;
	l.max = 8;
	d.vram = 4 * 1024 * 1024;
	h_eq_u64("4 Mio : 3 modes", dispi_list(&d, &l), 3);
	d.vram = 16777216;
	d.boot_w = 1366;
	d.boot_h = 768;
	h_eq_u64("mode de demarrage ajoute", dispi_list(&d, &l), 6);
	h_eq_u64("mode de demarrage en dernier", m[5].width, 1366);
}

int	main(void)
{
	h_begin("a13/dispi_mode");
	h_run("dispi table de decision des modes", decision_table);
	h_run("dispi limites de la vram", vram_limits);
	h_run("dispi liste de base", list_basic);
	h_run("dispi liste variantes", list_variants);
	return (h_end());
}
