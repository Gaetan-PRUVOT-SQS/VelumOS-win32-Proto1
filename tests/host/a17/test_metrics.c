#include "harness.h"
#include "velum/theme.h"
#include "luna_int.h"

static void	metrics_positive(void)
{
	t_lunametrics	m;
	const int32_t	*v;
	int				i;

	luna_metrics(&m);
	v = (const int32_t *)&m;
	i = 0;
	while (i < (int)(sizeof(m) / sizeof(int32_t)))
	{
		if (i != 1 && i != 2)
			h_true(v[i] > 0, "champ de metrique strictement positif");
		else
			h_true(v[i] >= 0, "bordure positive ou nulle");
		i++;
	}
}

static void	metrics_design(void)
{
	t_lunametrics	m;

	luna_metrics(&m);
	h_eq_i64("barre de titre", m.caption_h, VT_CAPTION_H);
	h_eq_i64("bouton de legende, largeur", m.btn_w, VT_CAPBTN_W);
	h_eq_i64("bouton de legende, hauteur", m.btn_h, VT_CAPBTN_H);
	h_eq_i64("espace reserve a la barre",
		m.taskbar_h, VT_DOCK_H + 2 * VT_DOCK_MARGIN);
	h_eq_i64("orbe du lanceur", m.start_w, VT_LAUNCHER_D);
	h_eq_i64("bordures nulles", m.frame_w + m.frame_bottom, 0);
	h_eq_i64("grande icone", m.icon_large, 56);
	h_eq_i64("icone de barre des taches", m.icon_small, VT_TASK_ICON);
	h_true(m.scroll_w % 2 == 1, "ascenseur impair : fleche centree");
	h_true(m.menu_item_h % 2 == 0, "entree de menu paire");
	h_true((m.taskbar_h - m.icon_small) % 2 == 0, "icone centree en barre");
}

static void	metrics_titlebar(void)
{
	t_lunametrics		m;
	const t_lunadetail	*d;

	luna_metrics(&m);
	d = lm_detail();
	h_true(m.caption_h > m.frame_w, "titre plus haut que la bordure");
	h_true(d->btn_top + m.btn_h <= m.caption_h, "bouton dans la barre");
	h_true(d->btn_right >= m.frame_w, "bouton hors de la bordure droite");
	h_true(d->btn_top >= m.frame_w, "bouton sous la bordure haute");
	h_true(d->btn_top_max + m.btn_h <= m.caption_h - m.frame_w, "max");
	h_true(d->tool_btn_top + d->tool_btn <= d->tool_caption_h, "outil");
	h_true(d->icon_left + d->caption_icon <= d->sysmenu_w,
		"icone dans la zone systeme");
	h_true(d->btn_gap > 0 && d->btn_gap < m.btn_w, "ecart de boutons");
	h_eq_i64("capsule centree dans la barre",
		d->btn_top * 2 + m.btn_h, m.caption_h);
	h_eq_i64("zone de redimensionnement", d->resize, VT_RESIZE_ZONE);
	h_eq_i64("rayon des grandes fenetres", d->title_radius, VT_R_WINDOW);
}

static void	metrics_stable(void)
{
	t_lunametrics	a;
	t_lunametrics	b;

	luna_metrics(&a);
	luna_metrics(&b);
	h_true(a.caption_h == b.caption_h && a.taskbar_h == b.taskbar_h,
		"deux appels, memes valeurs");
	luna_metrics(NULL);
	h_true(1, "luna_metrics(NULL) sans effet");
}

int	main(void)
{
	h_begin("a17/metrics");
	h_run("metrique positive", metrics_positive);
	h_run("valeurs de la maquette", metrics_design);
	h_run("boutons dans la barre de titre", metrics_titlebar);
	h_run("stabilite", metrics_stable);
	return (h_end());
}
