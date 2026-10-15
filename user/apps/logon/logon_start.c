#include "velum/abi/abi_syscall.h"
#include "../common/platform.h"
#include "../common/typo.h"
#include "logon.h"

static void	run(t_logon *lg)
{
	t_uimsg	m;
	int		n;
	int		w;

	while (!lg->quit)
	{
		n = ui_next(&m);
		while (n > 0)
		{
			logon_event(lg, &m);
			n = ui_next(&m);
		}
		if (n < 0)
			return ;
		w = ui_wait(lg->timer, TIMEOUT_INF);
		if (w == UI_WAIT_TIMER)
			logon_tick(lg);
		else if (w != UI_WAIT_MSG)
			return ;
	}
}

int	main(void)
{
	t_logon	lg;

	if (logon_boot(&lg) < 0)
	{
		os_log("logon: démarrage impossible (serveur de fenêtres" NBSP "?)");
		return (1);
	}
	os_log("logon: écran de connexion prêt");
	run(&lg);
	uiwin_close(&lg.ui);
	return (0);
}
