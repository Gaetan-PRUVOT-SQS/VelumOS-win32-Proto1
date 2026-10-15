#include "th.h"

void	th_boot_ready(void)
{
	t_inpevent	ev;

	th_ps2_reset();
	ps2_boot();
	th_irq();
	input_pop(&ev);
}

int	th_kbd_in(const uint8_t *b, uint32_t n, t_inpevent *ev, int max)
{
	f8042_kbd_bytes(b, n);
	th_irq();
	return (th_pop_all(ev, max));
}

int	th_mouse_in(const uint8_t *b, uint32_t n, t_inpevent *ev, int max)
{
	f8042_mouse_bytes(b, n);
	th_irq();
	return (th_pop_all(ev, max));
}
