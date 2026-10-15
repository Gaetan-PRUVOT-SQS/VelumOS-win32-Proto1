#ifndef FAKE_8042_H
# define FAKE_8042_H

# include <stdint.h>
# include "../../../drivers/input/ps2.h"

# define F8_FIFO 64
# define F8_LOG 128

typedef struct s_f8042
{
	uint8_t		out[F8_FIFO];
	uint8_t		aux[F8_FIFO];
	uint8_t		err[F8_FIFO];
	uint32_t	head;
	uint32_t	tail;
	uint8_t		cfg;
	uint8_t		pending;
	uint8_t		kbd_arg;
	uint8_t		mouse_arg;
	uint8_t		kbd_present;
	uint8_t		mouse_present;
	uint8_t		dual;
	uint8_t		wheel_ok;
	uint8_t		rates[3];
	uint8_t		mouse_id;
	uint8_t		scanset;
	uint8_t		leds;
	uint8_t		kbd_scanning;
	uint8_t		mouse_stream;
	uint8_t		selftest;
	uint8_t		porttest[2];
	uint8_t		float_bus;
	uint8_t		stuck_ibf;
	uint8_t		selftest_resets;
	uint8_t		nak_kbd;
	uint8_t		kbd_silent;
	uint8_t		mouse_silent;
	uint8_t		cmds[F8_LOG];
	uint32_t	ncmds;
	uint8_t		kbd_log[F8_LOG];
	uint32_t	nkbd;
	uint8_t		mouse_log[F8_LOG];
	uint32_t	nmouse;
}	t_f8042;

extern t_f8042	g_f8042;

void			f8042_reset(void);
void			f8042_push(uint8_t b, uint8_t aux);
void			f8042_push_err(uint8_t b, uint8_t aux);
uint8_t			f8042_status(void);
uint8_t			f8042_read(void);
void			f8042_write_cmd(uint8_t v);
void			f8042_write_data(uint8_t v);
const t_ps2ops	*f8042_ops(void);
void			f8042_ctl_cmd(uint8_t c);
void			f8042_kbd_in(uint8_t v);
void			f8042_mouse_in(uint8_t v);
void			f8042_kbd_bytes(const uint8_t *b, uint32_t n);
void			f8042_mouse_bytes(const uint8_t *b, uint32_t n);
uint32_t		f8042_pending(void);

#endif
