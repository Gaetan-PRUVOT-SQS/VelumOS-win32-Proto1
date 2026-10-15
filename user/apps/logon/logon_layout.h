#ifndef LOGON_LAYOUT_H
# define LOGON_LAYOUT_H

# include <stdint.h>
# include "velum/gfx.h"
# include "velum/luna.h"

# define LOGON_MIN_W 640
# define LOGON_MIN_H 480
# define LOGON_TILES_MAX 6
# define LOGON_CARD_W 380
# define LOGON_PAD_TOP 28
# define LOGON_PAD_SIDE 24
# define LOGON_PAD_BOTTOM 24
# define LOGON_GAP 16
# define LOGON_MARGIN 16
# define LOGON_AVATAR 88
# define LOGON_AVATAR_ICON 68
# define LOGON_AVATAR_ROW 48
# define LOGON_NAME_H 26
# define LOGON_TILE_H 56
# define LOGON_TILE_GAP 8
# define LOGON_PW_H 48
# define LOGON_PW_PAD 18
# define LOGON_GO_D 38
# define LOGON_GO_W 38
# define LOGON_GO_INSET 5
# define LOGON_ROW_H 28
# define LOGON_MSG_H 28
# define LOGON_SHUT_W 96
# define LOGON_SHUT_H 28
# define LOGON_CLOCK_Y 100
# define LOGON_CLOCK_H 120
# define LOGON_CLOCK_MIN_TOP 16
# define LOGON_DATE_GAP 6
# define LOGON_DATE_H 28
# define LOGON_CARD_GAP 57
# define LOGON_HEAD_H 211
# define LOGON_BAND_TOP 8
# define LOGON_BAND_BOTTOM 88
# define LOGON_DLG_TEXT_H 60
# define LOGON_DLG_BTN_W 96
# define LOGON_DLG_BTN_H 28
# define LOGON_RING LUNA_FOCUS_MARGIN
# define LOGON_CHIP_H 44
# define LOGON_CHIP_MARGIN 28
# define LOGON_CHIP_ICON 32
# define LOGON_CHIP_PAD 6
# define LOGON_CHIP_GAP 8
# define LOGON_CHIP_TAIL 16
# define LOGON_CHIP_SPACE 10
# define LOGON_CHIP_USER "Changer d'utilisateur"
# define LOGON_CHIP_LANG "FR"

typedef struct s_logonlayout
{
	uint32_t	count;
	t_rect		prompt;
	t_rect		clock;
	t_rect		date;
	t_rect		card;
	t_rect		avatar;
	t_rect		name;
	t_rect		tile[LOGON_TILES_MAX];
	t_rect		field;
	t_rect		password;
	t_rect		go;
	t_rect		message;
	t_rect		shutdown;
	t_rect		dlg_text;
	t_rect		dlg_ok;
	t_rect		dlg_cancel;
	t_rect		chips;
}	t_logonlayout;

int32_t		logon_ident_h(uint32_t count);
int32_t		logon_card_h(uint32_t count);
uint32_t	logon_tiles_fit(int32_t h, uint32_t count);
void		logon_place_card(t_logonlayout *o, int32_t w, int32_t h);
void		logon_place_dialog(t_logonlayout *o);
void		logon_place_chips(t_logonlayout *o, int32_t w, int32_t h);
t_rect		logon_btn_rect(int32_t x, int32_t y, int32_t w, int32_t h);
int			logon_layout(int32_t w, int32_t h, uint32_t count,
				t_logonlayout *out);

#endif
