#include "obj_int.h"

uint64_t	wait_deadline(uint64_t timeout)
{
	uint64_t	now;

	if (timeout == TIMEOUT_INF)
		return (TIMEOUT_INF);
	now = time_now_ns();
	if (timeout >= TIMEOUT_INF - now)
		return (TIMEOUT_INF);
	return (now + timeout);
}

int	obj_wait(t_object *obj, uint64_t timeout_ns)
{
	int	rc;

	if (!obj)
		return (E_INVAL);
	rc = wait_objects(&obj, 1, WAIT_ANY, timeout_ns);
	if (rc > 0)
		rc = 0;
	return (rc);
}

int	wait_for(t_object *obj, uint64_t deadline)
{
	uint64_t	now;

	if (deadline == TIMEOUT_INF)
		return (obj_wait(obj, TIMEOUT_INF));
	now = time_now_ns();
	if (now >= deadline)
		return (E_TIMEOUT);
	return (obj_wait(obj, deadline - now));
}
