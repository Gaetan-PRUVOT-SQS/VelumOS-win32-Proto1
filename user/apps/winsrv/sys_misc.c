#include "velum/vmisc.h"
#include "velum/vtime.h"
#include "ws_sys.h"

uint64_t	ws_sys_now(void)
{
	int64_t	t;

	t = v_time_mono();
	if (t < 0)
		return (0);
	return ((uint64_t)t);
}

void	ws_sys_log(const char *msg)
{
	v_log(V_LOG_INFO, msg);
}
