#include "velum/abi/abi_syscall.h"
#include "../common/platform.h"
#include "logon.h"

void	logon_start_session(t_logon *lg)
{
	const char	*name;
	int			r;

	name = lg->set.list[lg->shown[lg->flow.selected]].name;
	wmc_set_state(&lg->ui.win, WSTATE_HIDDEN);
	r = os_spawn(LOGON_SHELL_PATH, name, PF_SPAWN | PF_POWER, &lg->session);
	if (r == 0)
	{
		os_log("logon: session ouverte");
		os_wait_exit(lg->session);
		os_close(lg->session);
		os_log("logon: session fermée");
	}
	else
		os_log("logon: lancement du shell impossible");
	lf_session_ended(&lg->flow);
	if (r < 0)
		lg->flow.msg = LMSG_ERROR;
	wmc_set_state(&lg->ui.win, WSTATE_NORMAL);
	wmc_activate(lg->ui.win.id);
	logon_sync(lg);
	wmc_present(&lg->ui.win, NULL, 0);
}

void	logon_do_act(t_logon *lg)
{
	int	act;

	act = lg->act;
	lg->act = 0;
	if (act == LA_START_SESSION)
		logon_start_session(lg);
	else if (act == LA_POWER_OFF && os_power(POWER_OFF) < 0)
	{
		lf_cancel(&lg->flow);
		lg->flow.msg = LMSG_POWER;
		logon_sync(lg);
	}
}
