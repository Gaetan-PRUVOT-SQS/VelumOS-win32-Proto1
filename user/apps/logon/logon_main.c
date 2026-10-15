#include "velum/abi/abi_input.h"
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "logon.h"

int	logon_boot(t_logon *lg)
{
	int	r;

	memset(lg, 0, sizeof(*lg));
	lf_init(&lg->flow);
	lim_init(&lg->lim);
	logon_load(lg);
	r = ui_connect(&lg->info);
	if (r < 0)
		return (r);
	if (os_timer_open(&lg->timer) < 0)
		return (E_NOMEM);
	if (lg->nshown == 0)
		lg->flow.msg = LMSG_EMPTY;
	return (logon_window(lg));
}

void	logon_event(t_logon *lg, const t_uimsg *m)
{
	const t_wmkey	*k;
	const t_wmhdr	*h;

	h = (const t_wmhdr *)m->words;
	if (h->type == WMS_KEY && (size_t)m->len >= sizeof(*k)
		&& h->window == lg->ui.win.id)
	{
		k = (const t_wmkey *)m->words;
		if (k->ev.type == INP_KEY_DOWN && k->ev.code == VK_ESCAPE)
		{
			logon_after(lg, lf_cancel(&lg->flow));
			return ;
		}
	}
	uiwin_event(&lg->ui, m);
	logon_do_act(lg);
}
