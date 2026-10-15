#include "display_int.h"
#include "velum/arch.h"
#include "velum/libk.h"

#ifdef VELUM_DEBUG

bool	display_fault_requested(void)
{
	const char	*v;

	v = boot_cmdline_get("fault");
	return (v && !strcmp(v, "panic"));
}

#else

bool	display_fault_requested(void)
{
	return (false);
}

#endif

void	display_panic_screen(const char *t, const char *m)
{
	kcon_blue_screen(t, m);
	if (g_display.hold_on_panic)
		arch_halt_forever();
}
