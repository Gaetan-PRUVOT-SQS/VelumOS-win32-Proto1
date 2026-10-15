#include "ps2.h"
#include "../../kernel/input/inp_queue.h"
#include "../../kernel/input/xlate.h"
#include "velum/timer.h"

static void	ps2_key_byte(t_ps2 *g, uint8_t b, uint64_t now)
{
	t_keyraw	raw;
	t_inpevent	ev[KEY_OUT_MAX];
	uint8_t		locks;
	int			n;

	if (led_byte(g, b, now) || ps2_kbd_skip(g, b))
		return ;
	if (ps2_kbd_loss_byte(b))
	{
		ps2_kbd_lost(g);
		return ;
	}
	if (!scan2_feed(&g->scan, b, &raw))
		return ;
	n = xlate_key(&raw, ev, &locks);
	inpq_push_all(ev, n);
	if (locks != g->led.want)
		led_request(g, locks, now);
}

static void	ps2_mouse_byte(t_ps2 *g, uint8_t b, uint64_t now)
{
	t_mousepkt	pkt;
	t_inpevent	ev[MOUSE_OUT_MAX];
	int			n;

	if (!mousedec_feed(&g->mdec, b, now, &pkt))
		return ;
	n = xlate_mouse(&pkt, ev);
	inpq_push_all(ev, n);
}

static void	ps2_discard(t_ps2 *g, uint8_t st)
{
	if (st & PS2_ST_AUX)
		g->mdec.len = 0;
	else
		ps2_kbd_line_error(g);
}

void	ps2_drain(t_ps2 *g)
{
	uint8_t		st;
	uint8_t		b;
	uint64_t	now;
	int			n;

	now = time_now_ns();
	n = 0;
	while (n < PS2_DRAIN_MAX)
	{
		st = g->ops->status();
		if (!(st & PS2_ST_OBF))
			return ;
		b = g->ops->read();
		if (st & PS2_ST_ERR)
			ps2_discard(g, st);
		else if (st & PS2_ST_AUX && g->mouse_ok)
			ps2_mouse_byte(g, b, now);
		else if (!(st & PS2_ST_AUX) && g->kbd_ok)
			ps2_key_byte(g, b, now);
		n++;
	}
}

void	ps2_irq_handler(void *ctx)
{
	t_ps2		*g;
	uint64_t	flags;

	g = (t_ps2 *)ctx;
	flags = spin_lock_irqsave(&g->lock);
	ps2_drain(g);
	spin_unlock_irqrestore(&g->lock, flags);
}
