#include "auth.h"

int	auth_attempt(const t_authreq *rq, const char *pw, size_t pwlen,
		uint32_t *wait_s)
{
	const t_account	*a;
	uint32_t		left;

	*wait_s = 0;
	if (!rq->set || !rq->lim || rq->index >= rq->set->count)
		return (AUTH_DENIED);
	a = &rq->set->list[rq->index];
	if (a->flags & ACC_FLAG_DISABLED)
		return (AUTH_DENIED);
	left = lim_remaining_s(rq->lim, rq->index, rq->now_ns);
	if (left)
	{
		*wait_s = left;
		return (AUTH_WAIT);
	}
	if (acc_verify(a, pw, pwlen) == 1)
	{
		lim_reset(rq->lim, rq->index);
		return (AUTH_OK);
	}
	lim_fail(rq->lim, rq->index, rq->now_ns);
	return (AUTH_BAD);
}
