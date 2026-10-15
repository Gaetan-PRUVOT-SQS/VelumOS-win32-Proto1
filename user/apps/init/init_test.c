#include <string.h>
#include "init.h"
#include "velum/velum.h"

static int	session_ready(void)
{
	static t_procinfo	ps[INIT_PROCS_MAX];
	int					n;

	n = v_proc_list(ps, INIT_PROCS_MAX);
	if (n < 0)
		return (0);
	return (procs_have(ps, (uint32_t)n, "shell"));
}

void	killtest_setup(t_killtest *kt, int argc, char **argv)
{
	memset(kt, 0, sizeof(*kt));
	kt->state = KT_OFF;
	if (!killtest_requested(argc, argv))
		return ;
	kt->state = KT_ARMED;
	v_log(V_LOG_WARN, "init: essai mort-winsrv armé");
}

void	killtest_tick(t_killtest *kt, t_service *winsrv, uint64_t now_ns)
{
	int	fire;
	int	rc;

	if (kt->state != KT_ARMED && kt->state != KT_GRACE)
		return ;
	fire = killtest_step(kt, winsrv->h != 0 && session_ready(), now_ns);
	if (kt->state == KT_EXPIRED)
		v_log(V_LOG_WARN, "init: essai mort-winsrv abandonné à l'échéance");
	if (!fire)
		return ;
	rc = v_proc_kill(winsrv->h, KILLTEST_CODE);
	v_logf(V_LOG_WARN, "init: essai mort-winsrv, arrêt de %s (%d)",
		winsrv->path, rc);
}
