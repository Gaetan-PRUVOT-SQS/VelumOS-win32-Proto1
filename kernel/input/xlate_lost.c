#include "xlate.h"
#include "velum/timer.h"

int	xlate_keys_lost(t_inpevent *out)
{
	uint64_t	flags;
	uint64_t	now;
	int			n;
	int			i;

	flags = spin_lock_irqsave(&g_xlate.lock);
	n = kbd_release_held(&g_xlate.kbd, out);
	spin_unlock_irqrestore(&g_xlate.lock, flags);
	now = time_now_ns();
	i = 0;
	while (i < n)
	{
		out[i].time_ns = now;
		i++;
	}
	return (n);
}
