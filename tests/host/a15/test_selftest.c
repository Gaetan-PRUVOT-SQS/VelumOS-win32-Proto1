#include "harness.h"
#include "velum/gfx.h"

static void	selftest_noyau(void)
{
	h_eq_i64("gfx_selftest (autotest noyau)", gfx_selftest(), 0);
	h_eq_i64("gfx_selftest repetable", gfx_selftest(), 0);
}

int	main(void)
{
	h_begin("a15/selftest");
	h_run("autotest noyau exporte", selftest_noyau);
	return (h_end());
}
