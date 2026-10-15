#include "velum/velum.h"
#include "spin.h"

static int	spin_fail(const char *msg)
{
	v_log(V_LOG_ERR, msg);
	return (1);
}

int	spin_kill_test(const char *self)
{
	const char	*argv[3];
	int64_t		h;
	t_procinfo	pi;

	argv[0] = self;
	argv[1] = "loop";
	argv[2] = NULL;
	h = v_spawnv(self, argv, 0);
	if (h < 0)
		return (spin_fail("KILL FAIL spawn"));
	v_sleep(200000000ull);
	if (v_proc_kill((t_handle)h, 9) < 0)
		return (spin_fail("KILL FAIL kill"));
	if (v_wait((t_handle)h, 2000000000ull) < 0)
		return (spin_fail("KILL FAIL attente de la mort"));
	if (v_proc_info((t_handle)h, &pi) < 0 || pi.state != PS_ZOMBIE)
		return (spin_fail("KILL FAIL etat"));
	v_log(V_LOG_INFO, "KILL PASS");
	return (0);
}
