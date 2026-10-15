#include "logon_flow.h"

int	lf_session_ended(t_logonflow *f)
{
	if (f->mode != LM_SESSION)
		return (LA_NONE);
	f->mode = LM_PICK;
	f->back = LM_PICK;
	f->selected = LF_NO_ACCOUNT;
	f->msg = LMSG_NONE;
	f->wait_s = 0;
	return (LA_NONE);
}

int	lf_ask_shutdown(t_logonflow *f)
{
	if (f->mode != LM_PICK && f->mode != LM_PASSWORD)
		return (LA_NONE);
	f->back = f->mode;
	f->mode = LM_SHUTDOWN;
	return (LA_NONE);
}

int	lf_confirm_shutdown(t_logonflow *f)
{
	if (f->mode != LM_SHUTDOWN)
		return (LA_NONE);
	return (LA_POWER_OFF);
}
