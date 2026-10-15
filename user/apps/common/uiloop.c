#include "velum/err.h"
#include "platform.h"
#include "uiwin.h"

int	ui_next(t_uimsg *m)
{
	int	r;

	m->len = 0;
	r = wmc_next(m->words, sizeof(m->words), 0);
	if (r == E_TIMEOUT || r == E_PROTO)
		return (0);
	if (r < 0)
		return (r);
	m->len = r;
	return (r);
}

int	ui_wait(t_handle timer, uint64_t timeout_ns)
{
	if (timer == 0)
		return (os_wait1(wmc_channel(), timeout_ns));
	return (os_wait2(wmc_channel(), timer, timeout_ns));
}

int	ui_connect(t_wmhello *info)
{
	int	r;
	int	tries;

	r = wmc_connect(info);
	tries = 0;
	while (r < 0 && r != E_BUSY && tries < UI_CONNECT_TRIES)
	{
		os_sleep_ns(UI_CONNECT_STEP_NS);
		r = wmc_connect(info);
		tries++;
	}
	return (r);
}
