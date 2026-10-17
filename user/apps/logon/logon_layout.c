#include "velum/err.h"
#include "velum/libk.h"
#include "../common/layout.h"
#include "logon_layout.h"

int32_t	logon_ident_h(uint32_t count)
{
	if (count <= 1)
		return (LOGON_AVATAR + LOGON_GAP + LOGON_NAME_H);
	return ((int32_t)count * LOGON_TILE_H
		+ (int32_t)(count - 1) * LOGON_TILE_GAP);
}

int32_t	logon_card_h(uint32_t count)
{
	return (LOGON_PAD_TOP + logon_ident_h(count) + LOGON_GAP + LOGON_PW_H
		+ LOGON_GAP + LOGON_ROW_H + LOGON_PAD_BOTTOM);
}

static int32_t	place_identity(t_logonlayout *o)
{
	int32_t		y;
	int32_t		iw;
	uint32_t	i;

	iw = o->card.w - 2 * LOGON_PAD_SIDE;
	y = LOGON_PAD_TOP;
	if (o->count <= 1)
	{
		o->avatar = lay_rect((o->card.w - LOGON_AVATAR) / 2, y, LOGON_AVATAR,
				LOGON_AVATAR);
		o->name = lay_rect(LOGON_PAD_SIDE, y + LOGON_AVATAR + LOGON_GAP, iw,
				LOGON_NAME_H);
		if (o->count == 1)
			o->tile[0] = logon_btn_rect(LOGON_PAD_SIDE, y, iw,
					logon_ident_h(1));
		return (y + logon_ident_h(1));
	}
	i = 0;
	while (i < o->count)
	{
		o->tile[i] = logon_btn_rect(LOGON_PAD_SIDE, y, iw, LOGON_TILE_H);
		y += LOGON_TILE_H + LOGON_TILE_GAP;
		i++;
	}
	return (y - LOGON_TILE_GAP);
}

static void	place_children(t_logonlayout *o)
{
	int32_t	iw;
	int32_t	y;

	iw = o->card.w - 2 * LOGON_PAD_SIDE;
	y = place_identity(o) + LOGON_GAP;
	o->field = lay_rect(LOGON_PAD_SIDE, y, iw, LOGON_PW_H);
	o->password = lay_rect(LOGON_PAD_SIDE + LOGON_PW_PAD, y,
			iw - LOGON_PW_PAD - LOGON_GO_D - 2 * LOGON_GO_INSET, LOGON_PW_H);
	o->go = logon_btn_rect(LOGON_PAD_SIDE + iw - LOGON_GO_INSET - LOGON_GO_D,
			y + LOGON_GO_INSET, LOGON_GO_D, LOGON_GO_D);
	y += LOGON_PW_H + LOGON_GAP;
	o->message = lay_rect(LOGON_PAD_SIDE, y, iw - LOGON_SHUT_W - 8,
			LOGON_ROW_H);
	o->shutdown = logon_btn_rect(LOGON_PAD_SIDE + iw - LOGON_SHUT_W, y,
			LOGON_SHUT_W, LOGON_ROW_H);
}

int	logon_layout(int32_t w, int32_t h, uint32_t count, t_logonlayout *out)
{
	if (!out || w < LOGON_MIN_W || h < LOGON_MIN_H)
		return (E_RANGE);
	memset(out, 0, sizeof(*out));
	out->count = logon_tiles_fit(h, count);
	logon_place_card(out, w, h);
	place_children(out);
	logon_place_dialog(out);
	logon_place_chips(out, w, h);
	return ((int)out->count);
}
