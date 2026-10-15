#include "ps2.h"
#include "../../kernel/input/inp_queue.h"
#include "../../kernel/input/xlate.h"

int	ps2_kbd_loss_byte(uint8_t b)
{
	return (b == PS2_K_OVERRUN || b == PS2_K_LINE_ERR || b == PS2_R_BAT_OK);
}

void	ps2_kbd_lost(t_ps2 *g)
{
	t_inpevent	ev[KBD_HELD_MAX];
	int			n;

	g->scan.state = S2_IDLE;
	g->scan.e1pos = 0;
	n = xlate_keys_lost(ev);
	inpq_push_all(ev, n);
}

void	ps2_kbd_line_error(t_ps2 *g)
{
	ps2_kbd_lost(g);
	g->scan.skip = 1;
}

int	ps2_kbd_skip(t_ps2 *g, uint8_t b)
{
	if (!g->scan.skip)
		return (0);
	g->scan.skip = 0;
	return (b != PS2_K_EXT && b != PS2_K_EXT1 && b != PS2_K_BREAK);
}
