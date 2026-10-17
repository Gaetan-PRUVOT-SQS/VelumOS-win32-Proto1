#ifndef HELP_H
# define HELP_H

# include <stdint.h>
# include "velum/err.h"
# include "fake_kern.h"
# include "ws_srv.h"

typedef struct s_hrlast
{
	int32_t		status;
	uint32_t	seq;
}	t_hrlast;

extern t_hrlast	g_hr;

int			hs_start(t_wsrv *s, uint32_t w, uint32_t h);
void		hs_stop(t_wsrv *s);
void		hs_pump(void *arg);
void		hs_settle(t_wsrv *s, t_handle c);
void		hs_input(t_wsrv *s);
t_handle	hr_connect(t_wsrv *s);
int			hr_send(t_wsrv *s, t_handle c, void *msg);
int			hr_recv(t_handle c, void *buf, t_handle *hx);
int			hr_hello(t_wsrv *s, t_handle c);
uint32_t	hr_create(t_wsrv *s, t_handle c, t_rect r, uint32_t style);
uint32_t	hr_drain(t_handle c, uint32_t type, void *last);
void		hr_arg(t_wmarg *m, uint32_t type, uint32_t window, uint32_t value);
uint64_t	hg_next(uint64_t *st);
uint32_t	hg_below(uint64_t *st, uint32_t n);
uint64_t	hg_seed(const char *suite);
int			hp_write(const char *name, const t_surface *s);
uint32_t	hp_hash(const t_surface *s);
int			hz_add(t_wtable *t, uint32_t style);
void		hz_screen(t_wtable *t);
int			hz_cmp(const t_wtable *t, const int *own, const uint32_t *code);
void		hz_paint(const t_wtable *t, int *owner, uint32_t *code);
int			hv_check(const void *msg, uint32_t len);
void		hv_hello(t_wmhello *m);
t_handle	hr_client(t_wsrv *s);
void		hs_point(t_wsrv *s, int32_t x, int32_t y);
void		hs_click(t_wsrv *s, int32_t x, int32_t y);
void		hs_drag(t_wsrv *s, t_point from, t_point to);
int32_t		hs_capbtn_x(int32_t win_x, int32_t win_w, int rank);
int32_t		hs_capbtn_y(int32_t win_y);
void		hs_key(t_wsrv *s, uint32_t vk, uint32_t mods);
int			hs_slot(const t_wsrv *s, uint32_t id);
uint32_t	hs_create(t_wsrv *s, t_handle c, t_rect r, uint32_t style);
void		hr_activate(t_wsrv *s, t_handle c, uint32_t id);
void		hr_destroy(t_wsrv *s, t_handle c, uint32_t id);
void		hr_value(t_wsrv *s, t_handle c, uint32_t type, uint32_t iv[2]);
int32_t		hr_error(t_handle c);
int			hs_one(t_wsrv *s, t_handle *c, uint32_t *id);
t_rect		hx_rect(uint64_t *st);
int			hx_own(const t_wsrv *s, int ci, uint64_t *st);
void		hx_present(t_wsrv *s, t_handle c, int slot, uint64_t *st);
void		hx_input(t_wsrv *s, uint64_t *st, uint32_t k);
void		hx_req(t_wsrv *s, const t_handle *cl, uint64_t *st);
int			hx_compare(t_wsrv *s, t_surface *ref);
uint32_t	hf_message(const t_wsrv *s, uint8_t *buf, uint64_t *st);
uint64_t	ha_glass_bytes(uint32_t width);
bool		ha_has_size(uint64_t size);
int			ha_start(t_wsrv *s, int64_t fail, uint32_t w, uint32_t h);
int			hc_frame(t_wsrv *s, t_rect dirty, t_rect in);

#endif
