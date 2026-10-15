#ifndef TH_H
# define TH_H

# include <stdint.h>
# include <string.h>
# include "fake.h"
# include "fake_8042.h"
# include "../../../drivers/input/mouse_dec.h"
# include "../../../drivers/input/ps2_api.h"
# include "../../../drivers/input/scan2.h"
# include "../../../kernel/input/inp_queue.h"
# include "../../../kernel/input/kbd.h"
# include "../../../kernel/input/mouse.h"
# include "../../../kernel/input/xlate.h"
# include "velum/kinput.h"

# define TH_MAX_EV 8
# define TH_KEYS 48
# define TH_S 0x01
# define TH_C 0x02
# define TH_A 0x04
# define TH_W 0x08
# define TH_CAPS 0x10
# define TH_N 0x20
# define TH_R 0x40
# define TH_X 0x80
# define TH_LN 0x02
# define TH_SH 0x10000
# define TH_AG 0x20000
# define TH_LC 0x04

typedef struct s_s2case
{
	const char	*name;
	uint8_t		in[10];
	uint8_t		n;
	uint8_t		nev;
	uint16_t	code[2];
	uint8_t		rel[2];
}	t_s2case;

# define TH_READERS 3

typedef struct s_rd
{
	t_reader	*r;
	uint64_t	taken;
	uint64_t	last;
	int			order_errors;
}	t_rd;

typedef struct s_trip
{
	uint8_t		buf[3000];
	t_keyraw	got[1000];
	uint16_t	want[1000];
}	t_trip;

typedef struct s_kcase
{
	const char	*name;
	uint16_t	pre[3];
	uint8_t		locks;
	uint16_t	key;
	uint8_t		n;
	uint8_t		vk;
	uint16_t	mods;
	uint32_t	ch;
	uint16_t	chmods;
}	t_kcase;

typedef struct s_deadcase
{
	const char	*name;
	uint32_t	step[4];
	uint32_t	out[3];
}	t_deadcase;

typedef struct s_mdcase
{
	const char	*name;
	uint8_t		in[4];
	uint8_t		wheel;
	int16_t		dx;
	int16_t		dy;
	int8_t		dz;
	uint8_t		btn;
}	t_mdcase;

typedef struct s_vkcase
{
	uint16_t	code;
	uint8_t		vk;
	uint8_t		vk_num;
}	t_vkcase;

typedef struct s_colcase
{
	const char		*name;
	const uint32_t	*exp;
	uint8_t			held;
	uint8_t			locks;
}	t_colcase;

typedef struct s_privcase
{
	const char	*name;
	uint32_t	num;
	uint64_t	a[3];
	uint32_t	flags;
	int64_t		exp;
}	t_privcase;

int			th_scan(t_scan2 *s, const uint8_t *b, int n, t_keyraw *out);
t_keyraw	th_raw(uint16_t code, int release, int repeat);
int			th_press(t_kbd *k, uint16_t code, t_inpevent *out);
int			th_release(t_kbd *k, uint16_t code, t_inpevent *out);
uint32_t	th_char(t_kbd *k, uint16_t code);
uint32_t	th_resolve(t_kbd *k, uint16_t code);
void		th_check_col(const t_layout *l, const uint16_t *pos,
				const t_colcase *c);
void		th_check_vk(const t_layout *l, const uint16_t *pos,
				const uint8_t *vk);
void		th_run_kcase(const t_layout *l, const t_kcase *c);
int			th_dead_step(t_kbd *k, uint32_t step, uint32_t *out);
void		th_run_dead(const t_deadcase *c);
void		th_run_mdcase(const t_mdcase *c);
int			th_layout_dups(const t_layout *l);
int			th_layout_collisions(const t_layout *l);
int32_t		th_accel_step(t_accel *a, int32_t dx, int32_t dy, int32_t *oy);
int			th_popcount(uint32_t v);
uint64_t	th_pushes(void);
uint64_t	th_spawn(void *(*fn)(void *), void *arg);
void		th_join(uint64_t t);
void		*th_producer(void *arg);
void		*th_consumer(void *arg);
t_handle	th_open(uint32_t kind, t_reader **r);
t_handle	th_other_handle(void);
int64_t		th_sys_setup(uint32_t kind);
void		th_ps2_reset(void);
void		th_irq(void);
int			th_pop_all(t_inpevent *ev, int max);
int			th_cmd_index(uint8_t cmd, int from);
void		th_boot_ready(void);
int			th_kbd_in(const uint8_t *b, uint32_t n, t_inpevent *ev, int max);
int			th_mouse_in(const uint8_t *b, uint32_t n, t_inpevent *ev, int max);
void		th_push_n(uint32_t type, uint32_t first, uint32_t count);
int			th_take_codes(t_reader *r, uint32_t *codes, uint32_t max);
uint64_t	th_rand(uint64_t *state);
uint64_t	th_seed(const char *suite);
t_inpevent	th_event(uint32_t type, uint32_t code);
int			th_ev_is(const t_inpevent *e, uint32_t type, uint32_t code);

#endif
