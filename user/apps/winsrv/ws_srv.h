#ifndef WS_SRV_H
# define WS_SRV_H

# include <stdbool.h>
# include <stdint.h>
# include "ws_core.h"
# include "ws_sys.h"

# define WS_CLIENT_MAX 32
# define WS_OUT_MAX 16
# define WS_RECV_BATCH 16
# define WS_INPUT_BATCH 32
# define WS_FRAME_NS 16666667ull
# define WS_SECTION_TOTAL 0x4000000ull
# define WS_DBLCLICK_NS 500000000ull
# define WS_DBLCLICK_DIST 4
# define WS_PAGE 4096ull
# define WCF_USED 0x01
# define WCF_HELLO 0x02
# define WCF_SUB 0x04
# define WCF_DEAD 0x08
# define WSO_DROP 0x01
# define WS_SRC_LISTEN -1
# define WS_SRC_INPUT -2
# define WS_LEAVE 0x80000000u

typedef struct s_wout
{
	uint64_t	msg[WM_MSG_MAX / 8];
	uint32_t	len;
	t_handle	handle;
}	t_wout;

typedef struct s_wclient
{
	t_handle	chan;
	uint32_t	serial;
	uint32_t	flags;
	uint32_t	head;
	uint32_t	nout;
	uint32_t	dropped;
	uint64_t	sec_bytes;
	t_wout		out[WS_OUT_MAX];
}	t_wclient;

typedef struct s_wmouse
{
	t_point		pos;
	uint32_t	buttons;
	int32_t		capture;
	bool		explicit_cap;
	int32_t		hover;
	int32_t		hot_slot;
	int32_t		press_slot;
	uint32_t	press_ht;
	int32_t		click_slot;
	uint64_t	click_ns;
	t_point		click_pos;
}	t_wmouse;

typedef struct s_wkeys
{
	uint32_t	n;
	uint32_t	pos;
	uint32_t	ids[WS_WIN_MAX];
}	t_wkeys;

typedef struct s_wsrv
{
	t_wtable	t;
	t_wclient	c[WS_CLIENT_MAX];
	t_wsrx		rx;
	t_wdirty	dirty;
	t_wmouse	m;
	t_wdrag		drag;
	t_wkeys		k;
	t_wcursor	cur;
	t_surface	back;
	t_surface	fb;
	uint32_t	*glass;
	uint64_t	glass_bytes;
	t_handle	listener;
	t_handle	input;
	int32_t		shell;
	uint32_t	serial;
	uint64_t	sec_total;
	uint64_t	now;
	uint64_t	next_frame;
	t_rect		work_set;
	uint32_t	rot;
	uint32_t	frames;
	uint32_t	violations;
	bool		quit;
}	t_wsrv;

typedef int	(*t_wsreq)(t_wsrv *s, int ci, const void *msg);

void		ws_setup(t_wsrv *s, t_rect screen);
int			ws_open(t_wsrv *s);
void		ws_close_all(t_wsrv *s);
int			ws_step(t_wsrv *s);
void		ws_run(t_wsrv *s);
uint64_t	ws_timeout(const t_wsrv *s);
void		ws_accept(t_wsrv *s);
void		ws_client_read(t_wsrv *s, int ci);
void		ws_client_drop(t_wsrv *s, int ci);
void		ws_reap(t_wsrv *s);
void		ws_dispatch(t_wsrv *s, int ci, t_wsrx *rx);
void		ws_hdr(t_wmhdr *h, uint32_t type, uint32_t size, uint32_t window);
void		ws_post(t_wsrv *s, int ci, const void *msg, uint32_t flags);
int			ws_post_handle(t_wsrv *s, int ci, const void *msg, t_handle h);
void		ws_flush(t_wsrv *s, int ci);
void		ws_flush_all(t_wsrv *s);
bool		ws_pending(const t_wsrv *s);
void		ws_error(t_wsrv *s, int ci, const void *msg, int status);
int			ws_req_hello(t_wsrv *s, int ci, const void *msg);
int			ws_req_create(t_wsrv *s, int ci, const void *msg);
int			ws_req_destroy(t_wsrv *s, int ci, const void *msg);
int			ws_req_title(t_wsrv *s, int ci, const void *msg);
int			ws_req_rect(t_wsrv *s, int ci, const void *msg);
int			ws_req_state(t_wsrv *s, int ci, const void *msg);
int			ws_req_cursor(t_wsrv *s, int ci, const void *msg);
int			ws_req_icon(t_wsrv *s, int ci, const void *msg);
int			ws_req_present(t_wsrv *s, int ci, const void *msg);
int			ws_req_activate(t_wsrv *s, int ci, const void *msg);
int			ws_req_workarea(t_wsrv *s, int ci, const void *msg);
int			ws_req_subscribe(t_wsrv *s, int ci, const void *msg);
int			ws_req_capture(t_wsrv *s, int ci, const void *msg);
int			ws_own(const t_wsrv *s, int ci, uint32_t id);
int			ws_target(const t_wsrv *s, int ci, uint32_t id);
int			ws_claim_shell(t_wsrv *s, int ci);
int			ws_section_attach(t_wsrv *s, int slot);
void		ws_section_release(t_wsrv *s, int slot);
int			ws_send_section(t_wsrv *s, int slot, uint32_t type, uint32_t seq);
void		ws_win_destroy(t_wsrv *s, int slot);
void		ws_activate(t_wsrv *s, int slot);
void		ws_state_apply(t_wsrv *s, int slot, uint32_t state);
void		ws_set_state(t_wsrv *s, int slot, uint32_t state);
void		ws_apply_rect(t_wsrv *s, int slot, t_rect r, bool final);
void		ws_workarea(t_wsrv *s);
bool		ws_listed(const t_wwin *w);
void		ws_notify(t_wsrv *s, int slot, uint32_t type);
void		ws_notify_all(t_wsrv *s);
int			ws_req_theme(t_wsrv *s, int ci, const void *msg);
void		ws_theme_send(t_wsrv *s, int ci);
void		ws_theme_set(t_wsrv *s, uint32_t dark, uint32_t reduced);
void		ws_theme_reset(void);
void		ws_mark(t_wsrv *s, t_rect r);
void		ws_mark_win(t_wsrv *s, int slot);
void		ws_cursor_set(t_wsrv *s, t_point pos, uint32_t shape);
void		ws_frame(t_wsrv *s);
void		ws_glass_open(t_wsrv *s);
void		ws_glass_close(t_wsrv *s);
void		ws_input(t_wsrv *s, const t_inpevent *ev);
void		ws_input_read(t_wsrv *s);
void		ws_mouse_move(t_wsrv *s, int32_t dx, int32_t dy);
void		ws_mouse_button(t_wsrv *s, uint32_t button, bool down);
void		ws_mouse_wheel(t_wsrv *s, int32_t delta);
void		ws_hover(t_wsrv *s);
void		ws_send_mouse(t_wsrv *s, int slot, uint32_t type, uint32_t extra);
void		ws_caption_down(t_wsrv *s, int slot, uint32_t ht);
void		ws_caption_up(t_wsrv *s);
void		ws_drag_begin(t_wsrv *s, int slot, uint32_t ht);
void		ws_drag_end(t_wsrv *s);
void		ws_release_input(t_wsrv *s, int slot);
void		ws_key(t_wsrv *s, const t_inpevent *ev);
void		ws_alttab(t_wsrv *s, bool back);

#endif
