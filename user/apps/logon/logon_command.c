#include "../common/platform.h"
#include "logon.h"

void	logon_on_command(void *c, uint32_t code, void *user)
{
	const t_ctl	*ctl;
	t_logon		*lg;

	ctl = c;
	lg = user;
	if (!lg)
		return ;
	if (code == CN_CLICKED && ctl->id >= LOGON_ID_TILE
		&& ctl->id < LOGON_ID_TILE + LOGON_TILES_MAX)
		logon_pick(lg, ctl->id - LOGON_ID_TILE);
	else if ((code == CN_CLICKED && ctl->id == LOGON_ID_GO)
		|| (code == CN_ENTER && ctl->id == LOGON_ID_PW))
		logon_submit(lg);
	else if (code == CN_CLICKED && ctl->id == LOGON_ID_SHUT)
		logon_after(lg, lf_ask_shutdown(&lg->flow));
	else if (code == CN_CLICKED && ctl->id == CTL_ID_OK
		&& lg->flow.mode == LM_SHUTDOWN)
		logon_after(lg, lf_confirm_shutdown(&lg->flow));
	else if (code == CN_CLICKED && ctl->id == CTL_ID_CANCEL)
		logon_after(lg, lf_cancel(&lg->flow));
}

void	logon_tick(t_logon *lg)
{
	uint32_t	left;

	left = logon_remaining(lg);
	lg->flow.wait_s = left;
	if (left == 0)
	{
		if (lg->flow.msg == LMSG_WAIT || lg->flow.msg == LMSG_BAD)
			lg->flow.msg = LMSG_NONE;
		os_timer_cancel(lg->timer);
	}
	logon_sync(lg);
}
