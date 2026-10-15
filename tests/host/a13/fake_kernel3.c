#include "fakes.h"
#include "velum/boot.h"

const char	*boot_build_id(void)
{
	return ("0123456789abcdef0123456789abcdef01234567");
}

const t_cpufeat	*cpu_features(void)
{
	return (&g_fk.cpu);
}

void	fake_reset(void)
{
	memset(boot_info_rw(), 0, sizeof(t_bootinfo));
	memset(&g_fk, 0, sizeof(g_fk));
	g_fk.cpu.pat = true;
	memset(&g_fl, 0, sizeof(g_fl));
	fake_font_reset();
	fake_vmm_reset();
	fake_proc_reset();
	fake_dispi_reset();
}
