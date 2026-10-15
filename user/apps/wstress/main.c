#include "velum/velum.h"
#include "wstress.h"

static int	run_cycles(int count, int report)
{
	int	i;
	int	r;

	i = 0;
	while (i < count)
	{
		r = wstress_cycle();
		if (r < 0)
		{
			v_logf(V_LOG_ERR, "WSTRESS FAIL cycle %d code %d libre_ko %u",
				i + 1, r, wstress_free_kb());
			return (r);
		}
		i++;
		if (report && i % report == 0)
			v_logf(V_LOG_INFO, "WSTRESS cycle %d libre_ko %u", i,
				wstress_free_kb());
	}
	return (0);
}

int	main(void)
{
	uint32_t	before;
	uint32_t	after;

	v_log(V_LOG_INFO, "WSTRESS start");
	if (v_spawn(WSTRESS_SERVER, NULL, 0, PF_DISPLAY | PF_INPUT | PF_LISTEN) < 0)
	{
		v_log(V_LOG_ERR, "WSTRESS FAIL serveur");
		return (1);
	}
	if (run_cycles(WSTRESS_WARMUP, 0) < 0)
		return (1);
	v_sleep(WSTRESS_SETTLE_NS);
	before = wstress_free_kb();
	if (run_cycles(WSTRESS_CYCLES, WSTRESS_REPORT) < 0)
		return (1);
	v_sleep(WSTRESS_SETTLE_NS);
	after = wstress_free_kb();
	v_logf(V_LOG_INFO, "WSTRESS PASS cycles %d avant %u apres %u",
		WSTRESS_CYCLES, before, after);
	while (1)
		v_sleep(WSTRESS_SETTLE_NS);
	return (0);
}
