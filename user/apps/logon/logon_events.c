#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "../common/timefmt.h"
#include "logon.h"

static int	attempt(t_logon *lg, const char *pw, size_t len)
{
	t_authreq	rq;
	uint32_t	wait;
	int			res;

	res = AUTH_DENIED;
	wait = 0;
	if (lg->flow.selected < lg->nshown)
	{
		rq = (t_authreq){&lg->set, &lg->lim, lg->shown[lg->flow.selected],
			os_mono_ns()};
		res = auth_attempt(&rq, pw, len, &wait);
	}
	return (lf_result(&lg->flow, res, wait));
}

static void	wipe(t_logon *lg)
{
	t_ctl	*edit;

	edit = ctl_find(&lg->ui.root, LOGON_ID_PW);
	if (edit)
	{
		ctl_set_text(&lg->ui.root, edit, "");
		secure_zero(edit->text, sizeof(edit->text));
	}
	ctl_clip_clear();
}

void	logon_after(t_logon *lg, int act)
{
	wipe(lg);
	if (logon_remaining(lg) > 0)
		os_timer_after(lg->timer, NS_PER_SEC, NS_PER_SEC);
	else
		os_timer_cancel(lg->timer);
	if (act == LA_START_SESSION || act == LA_POWER_OFF)
		lg->act = act;
	logon_sync(lg);
}

void	logon_pick(t_logon *lg, uint32_t tile)
{
	int	act;

	if (tile >= lg->nshown)
		return ;
	act = lf_select(&lg->flow, tile,
			acc_needs_password(&lg->set.list[lg->shown[tile]]));
	if (act == LA_TRY_EMPTY)
		act = attempt(lg, "", 0);
	logon_after(lg, act);
}

void	logon_submit(t_logon *lg)
{
	const t_ctl	*edit;
	int			act;

	edit = ctl_find(&lg->ui.root, LOGON_ID_PW);
	if (!edit || lg->flow.mode != LM_PASSWORD)
		return ;
	act = attempt(lg, edit->text, strnlen(edit->text, sizeof(edit->text)));
	logon_after(lg, act);
}
