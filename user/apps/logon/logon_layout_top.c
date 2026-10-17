#include "velum/libk.h"
#include "../common/layout.h"
#include "logon_layout.h"

t_rect	logon_btn_rect(int32_t x, int32_t y, int32_t w, int32_t h)
{
	return (lay_rect(x - LOGON_RING, y - LOGON_RING, w + 2 * LOGON_RING,
			h + 2 * LOGON_RING));
}

void	logon_place_chips(t_logonlayout *o, int32_t w, int32_t h)
{
	o->chips = lay_rect(LOGON_CHIP_MARGIN,
			h - LOGON_CHIP_MARGIN - LOGON_CHIP_H,
			w - 2 * LOGON_CHIP_MARGIN, LOGON_CHIP_H);
}

void	logon_place_dialog(t_logonlayout *o)
{
	int32_t	iw;
	int32_t	y;

	iw = o->card.w - 2 * LOGON_PAD_SIDE;
	y = LOGON_PAD_TOP + LOGON_DLG_TEXT_H + 10;
	o->dlg_text = lay_rect(LOGON_PAD_SIDE, LOGON_PAD_TOP, iw,
			LOGON_DLG_TEXT_H);
	o->dlg_ok = logon_btn_rect(LOGON_PAD_SIDE + iw - 2 * LOGON_DLG_BTN_W - 8,
			y, LOGON_DLG_BTN_W, LOGON_DLG_BTN_H);
	o->dlg_cancel = logon_btn_rect(LOGON_PAD_SIDE + iw - LOGON_DLG_BTN_W, y,
			LOGON_DLG_BTN_W, LOGON_DLG_BTN_H);
}

uint32_t	logon_tiles_fit(int32_t h, uint32_t count)
{
	int32_t	room;
	int32_t	fit;

	room = h - LOGON_BAND_TOP - LOGON_BAND_BOTTOM
		- (logon_card_h(0) - logon_ident_h(0));
	fit = (room + LOGON_TILE_GAP) / (LOGON_TILE_H + LOGON_TILE_GAP);
	return ((uint32_t)lay_clamp((int32_t)count, 0,
			lay_clamp(fit, 1, LOGON_TILES_MAX)));
}

void	logon_place_card(t_logonlayout *o, int32_t w, int32_t h)
{
	int32_t	card_h;
	int32_t	top;
	int32_t	x;

	card_h = logon_card_h(o->count);
	x = (w - LOGON_CARD_W) / 2;
	top = lay_clamp(h - LOGON_BAND_BOTTOM - LOGON_HEAD_H - card_h, -1,
			LOGON_CLOCK_Y);
	o->prompt = lay_rect(0, 0, w, LOGON_BAND_TOP);
	if (top < LOGON_CLOCK_MIN_TOP)
	{
		top = lay_clamp((h - LOGON_BAND_BOTTOM - card_h) / 2, LOGON_BAND_TOP,
				h);
		o->card = lay_rect(x, top, LOGON_CARD_W, card_h);
		return ;
	}
	o->clock = lay_rect(0, top, w, LOGON_CLOCK_H);
	o->date = lay_rect(0, top + LOGON_CLOCK_H + LOGON_DATE_GAP, w,
			LOGON_DATE_H);
	o->prompt = lay_rect(0, top, w,
			LOGON_CLOCK_H + LOGON_DATE_GAP + LOGON_DATE_H);
	o->card = lay_rect(x, top + LOGON_HEAD_H, LOGON_CARD_W, card_h);
}
