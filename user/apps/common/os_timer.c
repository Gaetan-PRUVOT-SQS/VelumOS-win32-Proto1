#include "velum/velum.h"
#include "platform.h"

int	os_timer_open(t_handle *out)
{
	int64_t	h;

	h = v_timer_create(0);
	if (h < 0)
		return ((int)h);
	*out = (t_handle)h;
	return (0);
}

int	os_timer_after(t_handle timer, uint64_t delay_ns, uint64_t period_ns)
{
	return (v_timer_set(timer, os_mono_ns() + delay_ns, period_ns));
}

int	os_timer_cancel(t_handle timer)
{
	return (v_timer_set(timer, 0, 0));
}

int	os_wait2(t_handle a, t_handle b, uint64_t timeout_ns)
{
	t_handle	h[2];

	h[0] = a;
	h[1] = b;
	return (v_wait_many(h, 2, WAIT_ANY, timeout_ns));
}
