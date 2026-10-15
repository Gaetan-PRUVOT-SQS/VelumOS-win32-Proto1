#include "vtest.h"

t_vtest	g_vtest;

void	vtest_check(int cond, const char *name)
{
	g_vtest.checks++;
	if (cond)
		return ;
	g_vtest.fails++;
	v_logf(V_LOG_ERR, "VTEST FAIL %s", name);
}

int	vtest_report(void)
{
	if (g_vtest.fails)
	{
		v_logf(V_LOG_ERR, "VTEST FAIL %d/%d", g_vtest.fails, g_vtest.checks);
		return (1);
	}
	v_logf(V_LOG_INFO, "VTEST PASS %d", g_vtest.checks);
	return (0);
}
