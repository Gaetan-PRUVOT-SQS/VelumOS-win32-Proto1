#ifndef WM_H
# define WM_H

# include <stdint.h>
# include "abi/abi_ipc.h"
# include "abi/abi_input.h"
# include "abi/abi_types.h"
# include "gfx.h"
# include "luna.h"

# define WM_PORT_NAME "wm"
# define WM_MAGIC 0x314d5756
# define WM_VERSION 1
# define WM_TITLE_MAX 64
# define WM_RECTS_MAX 16

# define WMC_HELLO 0x0001
# define WMC_CREATE 0x0002
# define WMC_DESTROY 0x0003
# define WMC_SET_TITLE 0x0004
# define WMC_SET_RECT 0x0005
# define WMC_SET_STATE 0x0006
# define WMC_PRESENT 0x0007
# define WMC_SET_CURSOR 0x0008
# define WMC_ACTIVATE 0x0009
# define WMC_SET_WORKAREA 0x000a
# define WMC_SUBSCRIBE 0x000b
# define WMC_SET_ICON 0x000c
# define WMC_CAPTURE 0x000d

# define WMS_HELLO_OK 0x1001
# define WMS_CREATED 0x1002
# define WMS_KEY 0x1003
# define WMS_MOUSE 0x1004
# define WMS_CLOSE_REQ 0x1005
# define WMS_ACTIVATE 0x1006
# define WMS_RESIZED 0x1007
# define WMS_PAINT 0x1008
# define WMS_STATE 0x1009
# define WMS_WIN_ADD 0x100a
# define WMS_WIN_DEL 0x100b
# define WMS_WIN_UPD 0x100c
# define WMS_ERROR 0x10ff

# define WSTATE_NORMAL 0
# define WSTATE_MIN 1
# define WSTATE_MAX 2
# define WSTATE_HIDDEN 3

# define CUR_ARROW 0
# define CUR_IBEAM 1
# define CUR_WAIT 2
# define CUR_SIZE_NS 3
# define CUR_SIZE_WE 4
# define CUR_SIZE_NWSE 5
# define CUR_SIZE_NESW 6
# define CUR_HAND 7

typedef struct s_wmhdr
{
	uint32_t	magic;
	uint32_t	type;
	uint32_t	size;
	uint32_t	window;
	uint32_t	seq;
	int32_t		status;
}	t_wmhdr;

typedef struct s_wmhello
{
	t_wmhdr		h;
	uint32_t	version;
	uint32_t	screen_w;
	uint32_t	screen_h;
	t_rect		workarea;
}	t_wmhello;

typedef struct s_wmcreate
{
	t_wmhdr		h;
	t_rect		rect;
	uint32_t	style;
	uint32_t	state;
	char		title[WM_TITLE_MAX];
}	t_wmcreate;

typedef struct s_wmcreated
{
	t_wmhdr		h;
	uint32_t	stride;
	uint32_t	width;
	uint32_t	height;
	uint32_t	reserved;
}	t_wmcreated;

typedef struct s_wmpresent
{
	t_wmhdr		h;
	uint32_t	nrects;
	t_rect		rects[WM_RECTS_MAX];
}	t_wmpresent;

typedef struct s_wmkey
{
	t_wmhdr		h;
	t_inpevent	ev;
}	t_wmkey;

typedef struct s_wmmouse
{
	t_wmhdr		h;
	uint32_t	type;
	int32_t		x;
	int32_t		y;
	uint32_t	buttons;
	int32_t		wheel;
	uint32_t	hit;
}	t_wmmouse;

typedef struct s_wminfo
{
	t_wmhdr		h;
	uint32_t	pid;
	uint32_t	style;
	uint32_t	state;
	uint32_t	icon;
	uint32_t	active;
	char		title[WM_TITLE_MAX];
}	t_wminfo;

typedef struct s_wmwin
{
	uint32_t	id;
	t_handle	chan;
	t_handle	section;
	t_surface	surface;
	uint32_t	style;
}	t_wmwin;

int			wmc_connect(t_wmhello *info);
int			wmc_create(t_wmwin *w, const t_wmcreate *rq);
int			wmc_present(t_wmwin *w, const t_rect *rects, uint32_t n);
int			wmc_set_title(t_wmwin *w, const char *title);
int			wmc_set_state(t_wmwin *w, uint32_t state);
int			wmc_set_rect(t_wmwin *w, t_rect r);
int			wmc_activate(uint32_t window);
int			wmc_next(void *buf, uint32_t size, uint64_t timeout_ns);
t_handle	wmc_channel(void);

# define WM_MSG_MAX 320
# define WM_BTN_MASK 0xff
# define WM_BTN_SHIFT 8
# define WM_SECTION_MAX 0x1000000
# define WM_WIN_PER_CLIENT 64
# define WM_WIN_TOTAL 128
# define WM_REPLY_NS 2000000000ull

typedef struct s_wmarg
{
	t_wmhdr		h;
	uint32_t	value;
	uint32_t	reserved;
}	t_wmarg;

typedef struct s_wmrect
{
	t_wmhdr		h;
	t_rect		rect;
}	t_wmrect;

typedef struct s_wmtitle
{
	t_wmhdr		h;
	char		title[WM_TITLE_MAX];
}	t_wmtitle;

int			wmc_destroy(t_wmwin *w);
int			wmc_set_cursor(t_wmwin *w, uint32_t cursor);
int			wmc_set_icon(t_wmwin *w, uint32_t icon);
int			wmc_capture(t_wmwin *w, uint32_t on);
int			wmc_set_state_id(uint32_t window, uint32_t state);
int			wmc_set_workarea(t_rect r);
int			wmc_subscribe(uint32_t on);
void		wmc_disconnect(void);

#endif
