#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "layout.h"
#include "taskbar.h"

static void	taskbar_regions_normales(void)
{
	t_tbmetrics	m;
	t_tbregions	r;

	m = (t_tbmetrics){1024, 80, 12, 8, 8, 42, 190, 42, 40};
	h_eq_i64("regions", tb_regions(&m, &r), 0);
	h_eq_i64("orbe x: marge 12 + remplissage 8", r.start.x, 20);
	h_eq_i64("orbe y: centre dans le dock 12..68", r.start.y, 19);
	h_eq_i64("orbe diametre", r.start.w, 42);
	h_eq_i64("orbe hauteur", r.start.h, 42);
	h_eq_i64("notification x: 1024 - 12 - 8 - 190", r.tray.x, 814);
	h_eq_i64("notification y", r.tray.y, 19);
	h_eq_i64("notification hauteur", r.tray.h, 42);
	h_eq_i64("bande x: orbe + 8 + 4 (maquette 4.1)", r.strip.x, 74);
	h_eq_i64("bande y: centree dans le dock", r.strip.y, 20);
	h_eq_i64("bande hauteur: taches de 40", r.strip.h, 40);
	h_eq_i64("bande largeur: 814 - 8 - 74", r.strip.w, 732);
	h_true(!lay_overlap(r.start, r.strip), "orbe hors bande");
	h_true(!lay_overlap(r.tray, r.strip), "notification hors bande");
	h_true(lay_inside(lay_rect(12, 12, 1000, 56), r.strip), "bande dans dock");
	h_true(lay_inside(lay_rect(12, 12, 1000, 56), r.tray), "zone dans dock");
	h_eq_i64("orbe: marges haute et basse egales", r.start.y - 12,
		68 - (r.start.y + r.start.h));
}

static void	taskbar_regions_invalides(void)
{
	t_tbmetrics	m;
	t_tbregions	r;

	m = (t_tbmetrics){1024, 24, 12, 8, 8, 42, 190, 42, 40};
	h_eq_i64("barre sans dock", tb_regions(&m, &r), -22);
	m = (t_tbmetrics){1024, 60, 12, 8, 8, 42, 190, 42, 40};
	h_eq_i64("dock plus bas que l'orbe", tb_regions(&m, &r), -22);
	m = (t_tbmetrics){1024, 80, 12, 8, 8, 0, 190, 42, 40};
	h_eq_i64("orbe nul", tb_regions(&m, &r), -22);
	m = (t_tbmetrics){1024, 80, 12, 8, 8, 42, -1, 42, 40};
	h_eq_i64("notification negative", tb_regions(&m, &r), -22);
	m = (t_tbmetrics){150, 80, 12, 8, 8, 42, 190, 42, 40};
	h_eq_i64("ecran trop etroit", tb_regions(&m, &r), -34);
	m = (t_tbmetrics){1024, 80, 12, 8, -1, 42, 190, 42, 40};
	h_eq_i64("interstice negatif", tb_regions(&m, &r), -22);
	m = (t_tbmetrics){1024, 80, 12, 8, 65, 42, 190, 42, 40};
	h_eq_i64("interstice trop grand", tb_regions(&m, &r), -22);
	m = (t_tbmetrics){100000, 80, 12, 8, 8, 42, 190, 42, 40};
	h_eq_i64("largeur hors bornes", tb_regions(&m, &r), -22);
	m = (t_tbmetrics){1024, 80, 12, 8, 0, 42, 190, 42, 40};
	h_eq_i64("interstice nul accepte", tb_regions(&m, &r), 0);
	h_eq_i64("sortie nulle", tb_regions(&m, NULL), -22);
}

static void	taskbar_cas_simples(void)
{
	t_rect	out[TASK_MAX];
	t_rect	strip;

	strip = lay_rect(74, 20, 732, 40);
	h_eq_i64("zero fenetre", tb_layout(strip, 0, out), 0);
	h_eq_i64("une fenetre", tb_layout(strip, 1, out), 1);
	h_eq_i64("largeur maximale", out[0].w, TB_BTN_MAX_W);
	h_eq_i64("une: x", out[0].x, 74);
	h_eq_i64("une: hauteur de tache", out[0].h, 40);
	h_eq_i64("trois fenetres", tb_layout(strip, 3, out), 3);
	h_eq_i64("trois: plafond", out[2].w, TB_BTN_MAX_W);
	h_eq_i64("trois: x du dernier", out[2].x, 74 + 2 * (160 + 6));
	h_eq_i64("cinq fenetres", tb_layout(strip, 5, out), 5);
	h_eq_i64("cinq: la bande est remplie", out[4].x + out[4].w, 806);
	h_eq_i64("quarante fenetres: (732+6)/(40+6)", tb_layout(strip, 40, out),
		16);
	h_eq_i64("bande vide", tb_layout(lay_rect(0, 0, 10, 40), 3, out), 0);
	h_eq_i64("hauteur nulle", tb_layout(lay_rect(0, 0, 736, 0), 3, out), 0);
}

static void	taskbar_test_de_clic(void)
{
	t_rect	out[TASK_MAX];

	tb_layout(lay_rect(74, 20, 732, 40), 3, out);
	h_eq_i64("premier bouton", tb_hit(out, 3, 74, 20), 0);
	h_eq_i64("dernier pixel du premier", tb_hit(out, 3, 233, 59), 0);
	h_eq_i64("interstice", tb_hit(out, 3, 234, 30), -1);
	h_eq_i64("second bouton", tb_hit(out, 3, 240, 30), 1);
	h_eq_i64("au dessus", tb_hit(out, 3, 90, 19), -1);
	h_eq_i64("en dessous", tb_hit(out, 3, 90, 60), -1);
	h_eq_i64("sous le dernier", tb_hit(out, 3, 700, 30), -1);
	h_eq_i64("aucun bouton", tb_hit(out, 0, 90, 30), -1);
}

int	main(void)
{
	h_begin("a20/taskbar");
	h_run("barre: regions normales", taskbar_regions_normales);
	h_run("barre: regions invalides", taskbar_regions_invalides);
	h_run("barre: cas simples 0, 1, 3, 5, 40", taskbar_cas_simples);
	h_run("barre: test de clic", taskbar_test_de_clic);
	return (h_end());
}
