#include "velum/libk.h"
#include "auth.h"
#include "logon_flow.h"

void	lf_init(t_logonflow *f)
{
	memset(f, 0, sizeof(*f));
	f->mode = LM_PICK;
	f->back = LM_PICK;
	f->selected = LF_NO_ACCOUNT;
}

int	lf_select(t_logonflow *f, uint32_t idx, int needs_password)
{
	if (f->mode != LM_PICK)
		return (LA_NONE);
	f->selected = idx;
	f->msg = LMSG_NONE;
	f->wait_s = 0;
	if (!needs_password)
		return (LA_TRY_EMPTY);
	f->mode = LM_PASSWORD;
	return (LA_NONE);
}

static void	fail_result(t_logonflow *f, int authres, uint32_t wait_s)
{
	f->msg = LMSG_BAD;
	f->wait_s = 0;
	if (authres == AUTH_WAIT)
	{
		f->msg = LMSG_WAIT;
		f->wait_s = wait_s;
	}
	if (authres == AUTH_DENIED)
	{
		f->msg = LMSG_ERROR;
		f->mode = LM_PICK;
		f->selected = LF_NO_ACCOUNT;
		return ;
	}
	f->mode = LM_PASSWORD;
}

int	lf_result(t_logonflow *f, int authres, uint32_t wait_s)
{
	if (f->mode != LM_PICK && f->mode != LM_PASSWORD)
		return (LA_NONE);
	if (f->selected == LF_NO_ACCOUNT)
		return (LA_NONE);
	if (authres == AUTH_OK)
	{
		f->mode = LM_SESSION;
		f->msg = LMSG_NONE;
		f->wait_s = 0;
		return (LA_START_SESSION);
	}
	fail_result(f, authres, wait_s);
	return (LA_NONE);
}

int	lf_cancel(t_logonflow *f)
{
	if (f->mode == LM_PASSWORD)
	{
		f->mode = LM_PICK;
		f->selected = LF_NO_ACCOUNT;
		f->msg = LMSG_NONE;
		f->wait_s = 0;
	}
	else if (f->mode == LM_SHUTDOWN)
		f->mode = f->back;
	return (LA_NONE);
}
