#ifndef FAKE_H
# define FAKE_H

# include <stdbool.h>
# include <stdint.h>
# include "velum/ctl.h"
# include "velum/err.h"

# define SENTINEL 0xdeadbeefu
# define FAKE_CHAR_W 6
# define FAKE_BOLD_W 7
# define FAKE_FONT_H 11
# define FK_MAX 2048
# define FK_BUTTON 1
# define FK_CHECK 2
# define FK_RADIO 3
# define FK_EDIT_FRAME 4
# define FK_GROUP 5
# define FK_PROGRESS 6
# define FK_MENU_PANEL 7
# define FK_MENU_ITEM 8
# define FK_TEXT 9
# define FAKE_CMD_MAX 256
# define FAKE_CB_MARK 0x10000

typedef struct s_fakemem
{
	int	live;
	int	calls;
	int	fail_at;
	int	failed;
}	t_fakemem;

typedef struct s_fakecall
{
	int		kind;
	t_rect	r;
	t_rect	clip;
	int		a;
	int		b;
	char	text[64];
}	t_fakecall;

typedef struct s_fakelog
{
	t_fakecall	calls[FK_MAX];
	int			n;
}	t_fakelog;

typedef struct s_fakecmd
{
	uint32_t	id;
	uint32_t	code;
	void		*ctl;
}	t_fakecmd;

typedef struct s_fakecmds
{
	t_fakecmd	list[FAKE_CMD_MAX];
	int			n;
	int			cbs;
}	t_fakecmds;

typedef struct s_fakeui
{
	t_surface	s;
	t_ctlroot	r;
}	t_fakeui;

extern t_fakemem	g_fake_mem;
extern int			g_fake_picked;
extern t_fakelog	g_log;
extern t_fakecmds	g_fake_cmds;
extern uint32_t		g_fake_buttons;
extern int32_t		g_fake_wheel;

void		fake_mem_reset(int fail_at);
void		fake_mem_arm(int fail_at);
int			fake_mem_live(void);
int			fake_mem_calls(void);
int			fake_mem_failed(void);
void		fake_log_add(const t_surface *s, int kind, t_rect r, int a);
void		fake_log_b(int b);
void		fake_log_text(const char *text, int len);
void		fake_log_clear(void);
int			fake_log_count(void);
t_fakecall	*fake_log_get(int i);
int			fake_log_kind(int kind);
t_fakecall	*fake_log_last(int kind);
void		fake_cmd(void *c, uint32_t code, void *user);
void		fake_cb(void *c, uint32_t code, void *user);
void		fake_cb_kill(void *c, uint32_t code, void *user);
void		fake_cb_destroy(void *c, uint32_t code, void *user);
void		fake_cb_pick(void *c, uint32_t code, void *user);
void		fake_cb_repop(void *c, uint32_t code, void *user);
void		fake_cmd_reset(void);
int			fake_cmd_count(void);
int			fake_cmd_code(int i);
int			fake_cmd_find(uint32_t id, uint32_t code);
int			fake_cmd_cbs(void);
void		fake_clock_set(int64_t ns);
int64_t		fake_clock_now(void);
int			fake_surf(t_surface *s, int w, int h);
void		fake_surf_free(t_surface *s);
int			fake_ui_open(t_fakeui *u, int w, int h);
void		fake_ui_close(t_fakeui *u);
void		fake_begin(t_fakeui *u, int w, int h);
void		fake_done(t_fakeui *u);
t_ctlspec	fake_spec(uint32_t type, uint32_t id, t_rect r, const char *text);
t_ctl		*fake_add(t_fakeui *u, t_ctlspec spec);
t_ctl		*fake_addp(t_fakeui *u, t_ctl *parent, t_ctlspec spec);
void		fake_item(t_ctlmenuitem *i, const char *t, uint32_t id, uint32_t f);
int			fake_scene(t_fakeui *u);
t_ctl		*fake_scene_menu(t_fakeui *u);
void		fake_dscene(t_fakeui *u, t_ctl **c);
t_ctl		*fake_btn(t_fakeui *u);
void		fake_dialog(t_fakeui *u);
bool		fake_focus_visible(t_fakeui *u, const t_ctl *c);
t_ctl		*fake_menu(t_fakeui *u, int x, int y);
int			fake_msel(const t_ctl *m, int lvl);
t_point		fake_mpt(const t_ctl *m, int lvl, int idx);
t_rect		fake_mrect(const t_ctl *m, int lvl, int idx);
t_rect		fake_mlevel(const t_ctl *m, int lvl);
t_ctl		*fake_chain(t_fakeui *u, t_ctlmenuitem *a, int n);
bool		fk_dn(t_fakeui *u, const t_ctl *m, int lvl, int idx);
bool		fk_mv(t_fakeui *u, const t_ctl *m, int lvl, int idx);
int			fake_mdepth(const t_ctl *m);
t_ctl		*fake_menu_id(t_fakeui *u, int x, int y, uint32_t id);
t_ctl		*fake_bar(t_fakeui *u, int max, int page);
t_ctl		*fake_list(t_fakeui *u, int items);
void		fake_fuzz_step(t_fakeui *u, t_ctl *e);
void		fake_storm_step(t_fakeui *u);
void		fake_poke(t_fakeui *u, t_ctl *c);
void		fake_storm_props(t_fakeui *u);
bool		fake_storm_ok(const t_fakeui *u);
bool		fake_fuzz_ok(const t_ctl *e);
t_ctl		*fake_edit(t_fakeui *u, const char *text);
void		fake_keys(t_ctlroot *r, uint32_t code, uint32_t mods, int times);
void		fake_fscene(t_fakeui *u);
int			fake_tabs(t_fakeui *u, int *out, bool forward);
void		fake_apply(t_fakeui *u, const int (*m)[3]);
t_ctl		*fk_c(t_fakeui *u, uint32_t type, uint32_t id, t_rect r);
bool		fk_no(t_fakeui *u, t_ctlspec sp);
void		fake_check_tree(t_fakeui *u, int items);
bool		fake_press(t_ctlroot *r, uint32_t code, uint32_t mods);
bool		fake_char(t_ctlroot *r, uint32_t cp);
int			fake_type(t_ctlroot *r, const char *utf8);
bool		fake_mouse(t_ctlroot *r, uint32_t type, int x, int y);
bool		fake_down(t_ctlroot *r, int x, int y);
bool		fake_up(t_ctlroot *r, int x, int y);
bool		fake_move(t_ctlroot *r, int x, int y);
bool		fake_wheel(t_ctlroot *r, int x, int y, int delta);
bool		fake_click(t_ctlroot *r, int x, int y);
bool		fake_utf8_valid(const char *s);
bool		fake_utf8_boundary(const char *s, uint32_t pos);
uint32_t	fake_utf8_chars(const char *s);
uint32_t	fake_rand(void);
void		fake_seed(uint32_t seed);
int			fake_px_count(const t_surface *s, t_rect in, uint32_t color);
bool		fake_dirty_is(const t_ctlroot *r, const t_rect *want, int n);
bool		fake_dirty_covers(const t_ctlroot *r, t_rect q);
void		fake_px_fill(t_surface *s, uint32_t color);
bool		fake_rect_eq(t_rect a, t_rect b);
t_color		fake_menu_hot_bg(void);
double		fake_contrast(t_color a, t_color b);
int			fake_band_area(t_rect r);

#endif
