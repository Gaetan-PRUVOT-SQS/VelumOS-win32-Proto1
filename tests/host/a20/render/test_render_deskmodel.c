#include <stdlib.h>
#include "harness.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static void	cellules_selon_le_modele(void)
{
	t_shell		sh;
	int32_t		rows;
	int32_t		i;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	rows = (SCREEN_H - sh.lm.taskbar_h - 24 - 81 + 99) / 99;
	h_true(rows >= 1, "au moins une rangee");
	h_eq_u64("trois icones placees", sh.placed, 3);
	i = 0;
	while (i < 3)
	{
		h_eq_i64("x = 24 + colonne * 90", sh.cells[i].x, 24 + i / rows * 90);
		h_eq_i64("y = 24 + rangee * 99", sh.cells[i].y, 24 + i % rows * 99);
		h_eq_i64("largeur 90", sh.cells[i].w, 90);
		h_eq_i64("hauteur 81", sh.cells[i].h, 81);
		i++;
	}
	h_eq_str("premiere icone", desk_icon(0)->label, "Ordinateur");
	h_eq_str("deuxieme icone", desk_icon(1)->label, "Corbeille");
	h_eq_str("troisieme icone", desk_icon(2)->label, "Bonjour");
	sh_shutdown(&sh);
}

static void	fond_suit_le_theme(void)
{
	t_shell		sh;
	t_surface	ref;
	uint32_t	*px;
	uint32_t	clair;
	t_point		pt;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	pt = (t_point){600, 300};
	clair = gfx_get(&sh.desk.surface, pt);
	px = malloc((size_t)SCREEN_W * SCREEN_H * sizeof(uint32_t));
	h_true(px != NULL, "tampon de reference");
	gfx_surface_init(&ref, px, SCREEN_W, SCREEN_H);
	luna_theme_set(LUNA_THEME_DARK);
	luna_wallpaper(&ref, rect_make(0, 0, SCREEN_W, SCREEN_H));
	sh_desk_paint(&sh);
	h_true(gfx_get(&sh.desk.surface, pt) != clair, "le fond a change");
	h_eq_u64("fond sombre", gfx_get(&sh.desk.surface, pt),
		px[pt.y * SCREEN_W + pt.x]);
	luna_theme_set(LUNA_THEME_LIGHT);
	sh_desk_cell(&sh, 0);
	h_eq_u64("retour au clair par une cellule", gfx_get(&sh.desk.surface, pt),
		clair);
	free(px);
	sh_shutdown(&sh);
}

static void	dialogues_suivent_la_legende(void)
{
	t_shell		sh;
	t_fwin		*w;
	t_lunawin	lw;
	t_rect		cl;
	t_rect		ok;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	drv_menu_item(&sh, SM_IDX_RUN);
	w = fwm_find("Exécuter");
	h_true(w != NULL, "dialogue ouvert");
	lw = (t_lunawin){rect_make(0, 0, w->rect.w, w->rect.h), w->style, "",
		ICON_PROGRAM, true, false, 0, 0};
	cl = luna_window_client(&lw);
	h_eq_i64("hauteur: legende + cadre + client de 124", w->rect.h,
		sh.lm.caption_h + sh.lm.frame_bottom + 124);
	ok = ctl_find(&sh.dlg.root, CTL_ID_CANCEL)->rect;
	h_true(ok.y + ok.h <= cl.h && ok.x + ok.w <= cl.w, "bouton dans le client");
	h_true(w->rect.y >= 0 && w->rect.y + w->rect.h
		<= SCREEN_H - sh.lm.taskbar_h, "dialogue dans la zone de travail");
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-deskmodel");
	h_run("bureau: cellules selon le modele", cellules_selon_le_modele);
	h_run("bureau: le fond suit le theme", fond_suit_le_theme);
	h_run("dialogues: taille selon la legende", dialogues_suivent_la_legende);
	return (h_end());
}
