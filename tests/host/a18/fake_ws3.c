#include <stdlib.h>
#include "velum/err.h"
#include "fake_kern.h"
#include "ws_sys.h"

int	ws_sys_kcon(uint32_t on)
{
	g_fk.kcon = on + 1;
	return (0);
}

int64_t	ws_sys_input_open(void)
{
	int		obj;
	int64_t	h;

	obj = fk_obj_new(FK_INPUT);
	if (obj < 0)
		return (obj);
	h = fk_handle_new(obj);
	if (h < 0)
		fk_unref(obj);
	return (h);
}

int	ws_sys_input_read(t_handle h, t_inpevent *ev, uint32_t max)
{
	uint32_t	n;

	if (fk_obj_of(h, FK_INPUT) < 0)
		return (E_BADF);
	n = 0;
	while (n < max && g_fk.in_n > 0)
	{
		ev[n] = g_fk.in[g_fk.in_head];
		g_fk.in_head = (g_fk.in_head + 1) % FK_INPUT_MAX;
		g_fk.in_n--;
		n++;
	}
	return ((int)n);
}

uint64_t	ws_sys_now(void)
{
	return (g_fk.now);
}
