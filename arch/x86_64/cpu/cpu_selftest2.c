#include "velum/err.h"
#include "velum/klog.h"
#include "cpu_int.h"

static t_fpubuf	g_fpubuf;

static int	st_int3(void)
{
	uint64_t	before;

	before = trap_breakpoints();
	cpu_trigger_int3();
	return (cpu_st_expect(trap_breakpoints() == before + 1, "int3"));
}

static void	st_vec_seen(t_regs *regs, void *ctx)
{
	*(uint64_t *)ctx = regs->vec;
}

static int	st_handler(void)
{
	uint64_t	seen;
	int			rc;
	int			fails;

	seen = 0;
	rc = idt_set_handler(CPU_TEST_VEC, st_vec_seen, &seen);
	if (rc == E_BUSY)
		return (0);
	fails = cpu_st_expect(rc == E_OK, "idt_set_handler");
	cpu_trigger_test_vec();
	fails += cpu_st_expect(seen == CPU_TEST_VEC, "gestionnaire appelé");
	rc = idt_set_handler(CPU_TEST_VEC, st_vec_seen, &seen);
	fails += cpu_st_expect(rc == E_BUSY, "double prise");
	rc = idt_set_handler(13, st_vec_seen, &seen);
	fails += cpu_st_expect(rc == E_INVAL, "exception réservée");
	idt_clear_handler(CPU_TEST_VEC);
	return (fails);
}

static int	st_fpu(void)
{
	uint8_t	*area;
	int		fails;

	area = g_fpubuf.area;
	if (cpu_st_expect(cpu_fpu_area_size() <= sizeof(g_fpubuf.area),
			"taille fpu"))
		return (1);
	cpu_fpu_init_state(area);
	cpu_fpu_restore(area);
	cpu_fpu_save(area);
	fails = cpu_st_expect(*(uint16_t *)area == FPU_FCW, "fpu fcw");
	fails += cpu_st_expect(*(uint32_t *)(area + FPU_MXCSR_OFF) == FPU_MXCSR,
			"fpu mxcsr");
	return (fails);
}

int	cpu_selftest_more(void)
{
	uint64_t	pcs[BT_MAX];
	int			n;
	int			fails;

	fails = st_int3() + st_handler() + st_fpu();
	n = cpu_backtrace((uint64_t)__builtin_frame_address(0), pcs, BT_MAX);
	fails += cpu_st_expect(n >= 2 && pcs[0] >= KERNEL_HALF, "trace");
	return (fails);
}
