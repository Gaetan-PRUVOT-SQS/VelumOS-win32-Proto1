#include <stdio.h>
#include "harness.h"
#include "lt_bbox.h"

static const int32_t	g_mh[] = {4, 5, 8, 11, 16, 24, 32, 48, 96};

static void	mark_height(t_lt *t, int32_t h)
{
	t_rect	box;
	t_rect	ink;
	char	name[96];

	lt_clear(t, LC_WHITE);
	box = lp_rect(20, 20, lp_wordmark_w(h), h);
	lp_wordmark(&t->s, box, LC_BLACK);
	ink = ltb_bbox(&t->s, NULL, LC_WHITE);
	snprintf(name, sizeof(name), "marque dessinee, hauteur %d", (int)h);
	h_true(ink.w > 0, name);
	snprintf(name, sizeof(name), "marque dans sa boite declaree, hauteur %d",
		(int)h);
	h_true(ltb_inside(ink, box), name);
	snprintf(name, sizeof(name), "marque occupe la largeur, hauteur %d",
		(int)h);
	h_true(ink.w >= box.w - 2 - h / 8, name);
}

static void	mark_sizes(void)
{
	t_lt	t;
	size_t	i;

	h_true(lt_open(&t, 700, 260) == 0, "allocation");
	i = 0;
	while (i < sizeof(g_mh) / sizeof(g_mh[0]))
	{
		mark_height(&t, g_mh[i]);
		i++;
	}
	h_true(lt_guard_ok(&t), "zones de garde intactes");
	lt_close(&t);
}

static void	emblem_shape(t_rect r, const char *what)
{
	t_lt	t;
	t_rect	ink;
	int32_t	side;
	char	name[96];

	h_true(lt_open(&t, 300, 300) == 0, "allocation");
	lt_clear(&t, LC_WHITE);
	lp_emblem(&t.s, r);
	ink = ltb_bbox(&t.s, NULL, LC_WHITE);
	side = r.w;
	if (r.h < side)
		side = r.h;
	snprintf(name, sizeof(name), "embleme dessine, %s", what);
	h_true(ink.w > 0, name);
	snprintf(name, sizeof(name), "embleme dans le carre central, %s", what);
	h_true(ltb_inside(ink, lp_rect(r.x + (r.w - side) / 2,
				r.y + (r.h - side) / 2, side, side)), name);
	snprintf(name, sizeof(name), "embleme centre, %s", what);
	h_true(ink.x + ink.w / 2 >= r.x + r.w / 2 - 2
		&& ink.x + ink.w / 2 <= r.x + r.w / 2 + 2, name);
	h_true(lt_guard_ok(&t), "zones de garde intactes");
	lt_close(&t);
}

static void	emblem_boxes(void)
{
	emblem_shape(lp_rect(40, 40, 120, 120), "carre");
	emblem_shape(lp_rect(10, 60, 240, 90), "large");
	emblem_shape(lp_rect(80, 10, 64, 250), "haut");
	emblem_shape(lp_rect(100, 100, 32, 32), "petit");
}

int	main(void)
{
	h_begin("a17/boot_marks");
	h_run("P8 marque : boite declaree, valeurs limites de hauteur",
		mark_sizes);
	h_run("P8 embleme : carre central de la boite", emblem_boxes);
	return (h_end());
}
