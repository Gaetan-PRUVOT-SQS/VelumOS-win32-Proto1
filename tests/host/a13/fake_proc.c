#include "fakes.h"
#include "velum/random.h"

t_fp	g_fp;

void	fake_proc_reset(void)
{
	memset(&g_fp, 0, sizeof(g_fp));
	g_fp.seed = 12345;
	fake_proc_set(10, PF_DISPLAY);
}

void	fake_proc_set(uint32_t pid, uint32_t flags)
{
	g_fp.proc.pid = pid;
	g_fp.proc.flags = flags;
	g_fp.proc.aspace = (t_aspace *)&g_fp.as_tag[pid & 1];
	g_fp.cur = &g_fp.proc;
}

t_process	*proc_current(void)
{
	return (g_fp.cur);
}

uint64_t	krandom_below(uint64_t bound)
{
	g_fp.randoms++;
	g_fp.last_bound = bound;
	g_fp.seed = g_fp.seed * 6364136223846793005ull + 1442695040888963407ull;
	if (!bound)
		return (0);
	return ((g_fp.seed >> 33) % bound);
}
