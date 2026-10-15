#ifndef PS2_H
# define PS2_H

# include <stdint.h>
# include "mouse_dec.h"
# include "scan2.h"
# include "velum/sync.h"

# define PS2_PORT_DATA 0x60
# define PS2_PORT_CMD 0x64
# define PS2_ST_OBF 0x01
# define PS2_ST_IBF 0x02
# define PS2_ST_AUX 0x20
# define PS2_ST_ERR 0xc0
# define PS2_CFG_IRQ1 0x01
# define PS2_CFG_IRQ2 0x02
# define PS2_CFG_CLK1_OFF 0x10
# define PS2_CFG_CLK2_OFF 0x20
# define PS2_CFG_XLAT 0x40
# define PS2_C_READ_CFG 0x20
# define PS2_C_WRITE_CFG 0x60
# define PS2_C_DIS2 0xa7
# define PS2_C_EN2 0xa8
# define PS2_C_TEST2 0xa9
# define PS2_C_SELFTEST 0xaa
# define PS2_C_TEST1 0xab
# define PS2_C_DIS1 0xad
# define PS2_C_EN1 0xae
# define PS2_C_WRITE2 0xd4
# define PS2_SELFTEST_OK 0x55
# define PS2_PORTTEST_OK 0x00
# define PS2_R_ACK 0xfa
# define PS2_R_RESEND 0xfe
# define PS2_R_BAT_OK 0xaa
# define PS2_K_OVERRUN 0x00
# define PS2_K_LINE_ERR 0xff
# define PS2_K_EXT 0xe0
# define PS2_K_EXT1 0xe1
# define PS2_K_BREAK 0xf0
# define PS2_D_RESET 0xff
# define PS2_D_DEFAULTS 0xf6
# define PS2_D_ENABLE 0xf4
# define PS2_K_SETLED 0xed
# define PS2_K_SCANSET 0xf0
# define PS2_M_SAMPLE 0xf3
# define PS2_M_GETID 0xf2
# define PS2_PORT_KBD 1
# define PS2_PORT_MOUSE 2
# define PS2_T_CTL 50000000ull
# define PS2_T_DEV 100000000ull
# define PS2_T_BAT 1000000000ull
# define PS2_T_LED 500000000ull
# define PS2_FLUSH_MAX 16
# define PS2_ACK_SCAN 8
# define PS2_DRAIN_MAX 32
# define PS2_RETRY 3
# define LED_IDLE 0
# define LED_ACK_CMD 1
# define LED_ACK_ARG 2

typedef struct s_ps2ops
{
	uint8_t	(*status)(void);
	uint8_t	(*read)(void);
	void	(*write_cmd)(uint8_t v);
	void	(*write_data)(uint8_t v);
}	t_ps2ops;

typedef struct s_led
{
	uint8_t		state;
	uint8_t		want;
	uint8_t		sent;
	uint8_t		tries;
	uint64_t	t0;
}	t_led;

typedef struct s_ps2
{
	t_spinlock		lock;
	const t_ps2ops	*ops;
	uint8_t			dual;
	uint8_t			kbd_ok;
	uint8_t			mouse_ok;
	uint8_t			mouse_id;
	t_scan2			scan;
	t_mousedec		mdec;
	t_led			led;
}	t_ps2;

extern t_ps2	g_ps2;

const t_ps2ops	*ps2_hw_ops(void);
int				ps2_wait(const t_ps2ops *o, uint8_t mask, uint8_t want,
					uint64_t timeout_ns);
int				ps2_ctl_write(const t_ps2ops *o, uint8_t cmd);
int				ps2_data_write(const t_ps2ops *o, uint8_t v);
int				ps2_data_read(const t_ps2ops *o, uint8_t *v,
					uint64_t timeout_ns);
void			ps2_flush(const t_ps2ops *o);
int				ps2_ctl_query(const t_ps2ops *o, uint8_t cmd, uint8_t *resp);
int				ps2_dev_send(t_ps2 *g, int port, uint8_t b);
int				ps2_wait_ack(t_ps2 *g);
int				ps2_dev_cmd(t_ps2 *g, int port, uint8_t cmd);
int				ps2_dev_cmd_arg(t_ps2 *g, int port, uint8_t cmd, uint8_t arg);
int				ps2_dev_reset(t_ps2 *g, int port, uint8_t *id);
int				i8042_cfg_read(t_ps2 *g, uint8_t *cfg);
int				i8042_cfg_write(t_ps2 *g, uint8_t cfg);
int				i8042_selftest(t_ps2 *g);
int				i8042_port_test(t_ps2 *g, uint8_t cmd);
int				i8042_init(t_ps2 *g);
int				ps2_kbd_init(t_ps2 *g);
int				ps2_mouse_init(t_ps2 *g);
void			led_request(t_ps2 *g, uint8_t mask, uint64_t now);
int				led_byte(t_ps2 *g, uint8_t b, uint64_t now);
void			ps2_drain(t_ps2 *g);
void			ps2_irq_handler(void *ctx);
int				ps2_kbd_loss_byte(uint8_t b);
void			ps2_kbd_lost(t_ps2 *g);
void			ps2_kbd_line_error(t_ps2 *g);
int				ps2_kbd_skip(t_ps2 *g, uint8_t b);
uint8_t			ps2_irq_hook(t_ps2 *g, uint8_t isa);
uint8_t			ps2_final_cfg(const t_ps2 *g, uint8_t cfg);

#endif
