#include "velum/err.h"
#include "velum/vdisplay.h"
#include "velum/vinput.h"
#include "velum/vobj.h"
#include "ws_sys.h"

int	ws_sys_display(t_dispinfo *di, void **fb)
{
	int64_t	va;
	int		r;

	r = v_display_info(di);
	if (r < 0)
		return (r);
	va = v_display_map(0);
	if (va <= 0)
		return (E_NODEV);
	*fb = (void *)(uintptr_t)va;
	return (0);
}

int	ws_sys_kcon(uint32_t on)
{
	return (v_kcon(on != 0));
}

int64_t	ws_sys_input_open(void)
{
	return (v_input_open(0));
}

int	ws_sys_input_read(t_handle h, t_inpevent *ev, uint32_t max)
{
	return (v_input_read(h, ev, max));
}

int	ws_sys_wait(const t_handle *hs, uint32_t n, uint64_t timeout_ns)
{
	return (v_wait_many(hs, n, WAIT_ANY, timeout_ns));
}
