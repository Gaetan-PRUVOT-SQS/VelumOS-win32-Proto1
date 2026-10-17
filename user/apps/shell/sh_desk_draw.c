#include "velum/libk.h"
#include "../common/uitext.h"
#include "sh_desk.h"
#include "shell.h"

static t_color	plate_color(void)
{
	if (luna_theme_get() == LUNA_THEME_DARK)
		return (SH_PLATE_DARK);
	return (SH_PLATE_LIGHT);
}

static void	draw_label(const t_cellctx *cx, const char *label)
{
	t_uitext	st;
	t_rrect		plate;
	int32_t		w;
	int32_t		fh;

	fh = ui_font_height(SH_LABEL_FONT);
	w = ui_text_width(SH_LABEL_FONT, label) + 2 * SH_LABEL_PAD_X;
	if (w > cx->cell.w)
		w = cx->cell.w;
	plate = (t_rrect){rect_make(cx->cell.x + (cx->cell.w - w) / 2, cx->cell.y
			+ SH_ICON_TOP + cx->sh->lm.icon_large + SH_LABEL_GAP, w,
			fh + 2 * SH_LABEL_PAD_Y), plate_color(), SH_LABEL_RADIUS,
		SH_LABEL_RADIUS, SH_LABEL_RADIUS, SH_LABEL_RADIUS};
	gfx_rrect_fill(&cx->sh->desk.surface, &plate);
	st.font = SH_LABEL_FONT;
	st.at = (t_point){plate.r.x + SH_LABEL_PAD_X, plate.r.y + SH_LABEL_TEXT_TOP};
	st.color = luna_color_text();
	st.max_w = w - 2 * SH_LABEL_PAD_X;
	ui_text(&cx->sh->desk.surface, &st, label);
}

static void	sel_paint(const t_cellctx *cx)
{
	t_selpaint	k;

	k.s = &cx->sh->desk.surface;
	k.cell = cx->cell;
	k.rgb = luna_color_accent() & 0x00ffffffu;
	k.ring_a = SH_SEL_RING_A;
	if (luna_theme_get() == LUNA_THEME_DARK)
		k.ring_a = SH_SEL_RING_A_DARK;
	sh_sel_paint(&k);
}

void	sh_desk_cell_draw(t_shell *sh, int32_t idx)
{
	const t_deskicon	*ic;
	t_cellctx			cx;
	t_rect				clip;
	t_point				at;

	ic = desk_icon((uint32_t)idx);
	if (!ic || (uint32_t)idx >= sh->placed)
		return ;
	cx = (t_cellctx){sh, sh->cells[idx], sh->selected == idx};
	clip = sh->desk.surface.clip;
	gfx_set_clip(&sh->desk.surface, cx.cell);
	if (cx.selected)
		sel_paint(&cx);
	at = (t_point){cx.cell.x + (cx.cell.w - sh->lm.icon_large) / 2,
		cx.cell.y + SH_ICON_TOP};
	sh_icon_shadow(&sh->desk.surface, at, (t_iconid)ic->icon,
		sh->lm.icon_large);
	luna_icon(&sh->desk.surface, at, (t_iconid)ic->icon, sh->lm.icon_large);
	draw_label(&cx, ic->label);
	sh->desk.surface.clip = clip;
}

void	sh_desk_cell(t_shell *sh, int32_t idx)
{
	if (idx < 0 || (uint32_t)idx >= sh->placed)
		return ;
	if (sh_wall_sync(sh))
	{
		sh_desk_paint(sh);
		return ;
	}
	sh_wall_blit(sh, sh->cells[idx]);
	sh_desk_cell_draw(sh, idx);
	wmc_present(&sh->desk, &sh->cells[idx], 1);
}
