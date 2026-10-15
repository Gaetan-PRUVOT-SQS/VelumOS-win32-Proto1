#include "ps2.h"

static void	led_start(t_ps2 *g, uint64_t now)
{
	g->led.tries = 0;
	g->led.state = LED_IDLE;
	if (ps2_data_write(g->ops, PS2_K_SETLED) < 0)
		return ;
	g->led.state = LED_ACK_CMD;
	g->led.t0 = now;
}

void	led_request(t_ps2 *g, uint8_t mask, uint64_t now)
{
	g->led.want = mask & 7;
	if (g->led.state != LED_IDLE && now - g->led.t0 <= PS2_T_LED)
		return ;
	led_start(g, now);
}

static int	led_ack(t_ps2 *g, uint64_t now)
{
	if (g->led.state == LED_ACK_CMD)
	{
		g->led.sent = g->led.want;
		g->led.state = LED_IDLE;
		if (ps2_data_write(g->ops, g->led.sent) == 0)
		{
			g->led.state = LED_ACK_ARG;
			g->led.t0 = now;
		}
		return (1);
	}
	g->led.state = LED_IDLE;
	if (g->led.sent != g->led.want)
		led_start(g, now);
	return (1);
}

static int	led_resend(t_ps2 *g)
{
	uint8_t	value;

	g->led.tries++;
	value = PS2_K_SETLED;
	if (g->led.state == LED_ACK_ARG)
		value = g->led.sent;
	if (g->led.tries > PS2_RETRY || ps2_data_write(g->ops, value) < 0)
		g->led.state = LED_IDLE;
	return (1);
}

int	led_byte(t_ps2 *g, uint8_t b, uint64_t now)
{
	if (g->led.state == LED_IDLE)
		return (0);
	if (now > g->led.t0 && now - g->led.t0 > PS2_T_LED)
	{
		g->led.state = LED_IDLE;
		return (0);
	}
	if (b == PS2_R_ACK)
		return (led_ack(g, now));
	if (b == PS2_R_RESEND)
		return (led_resend(g));
	return (0);
}
