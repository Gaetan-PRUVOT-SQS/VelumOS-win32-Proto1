#include "xlate.h"
#include "velum/timer.h"

t_xlate	g_xlate;

void	xlate_init(void)
{
	spin_init(&g_xlate.lock, "input_xlate");
	kbd_init(&g_xlate.kbd);
	mouse_init(&g_xlate.mouse);
}

static void	stamp(t_inpevent *ev, int n)
{
	uint64_t	now;
	int			i;

	now = time_now_ns();
	i = 0;
	while (i < n)
	{
		ev[i].time_ns = now;
		i++;
	}
}

int	xlate_key(const t_keyraw *raw, t_inpevent *out, uint8_t *locks)
{
	uint64_t	flags;
	int			n;

	flags = spin_lock_irqsave(&g_xlate.lock);
	n = kbd_translate(&g_xlate.kbd, raw, out);
	*locks = g_xlate.kbd.locks;
	spin_unlock_irqrestore(&g_xlate.lock, flags);
	stamp(out, n);
	return (n);
}

int	xlate_mouse(const t_mousepkt *p, t_inpevent *out)
{
	uint64_t	flags;
	int			n;

	flags = spin_lock_irqsave(&g_xlate.lock);
	n = mouse_events(&g_xlate.mouse, p, kbd_mods(&g_xlate.kbd), out);
	spin_unlock_irqrestore(&g_xlate.lock, flags);
	stamp(out, n);
	return (n);
}

uint8_t	xlate_locks(void)
{
	uint64_t	flags;
	uint8_t		locks;

	flags = spin_lock_irqsave(&g_xlate.lock);
	locks = g_xlate.kbd.locks;
	spin_unlock_irqrestore(&g_xlate.lock, flags);
	return (locks);
}
