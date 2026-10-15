#include "velum/libk.h"
#include "velum/panic.h"
#include "velum/util.h"
#include "cpu_int.h"

static t_fpustate	g_fpu;

void	cpu_fpu_boot(const t_cpuid_raw *raw)
{
	uint32_t	regs[4];
	uint64_t	xcr0;

	g_fpu.size = FXSAVE_SIZE;
	if (cpuid_bit(raw, CPUID_L1, R_ECX, 26)
		&& (raw->r[CPUID_LD][R_EAX] & (XCR0_X87 | XCR0_SSE)) == 3)
	{
		xcr0 = XCR0_X87 | XCR0_SSE;
		if (cpuid_bit(raw, CPUID_L1, R_ECX, 28)
			&& (raw->r[CPUID_LD][R_EAX] & XCR0_AVX))
			xcr0 |= XCR0_AVX;
		cpu_xsetbv(0, xcr0);
		cpu_cpuid(0xd, 0, regs);
		if (regs[R_EBX] >= FXSAVE_SIZE + FPU_ALIGN)
		{
			g_fpu.xsave = true;
			g_fpu.xcr0 = xcr0;
			g_fpu.size = align_up(regs[R_EBX], FPU_ALIGN);
		}
	}
	cpu_fpu_hw_reset();
}

uint64_t	cpu_fpu_area_size(void)
{
	return (g_fpu.size);
}

void	cpu_fpu_init_state(void *area)
{
	uint8_t	*bytes;

	kassert_check(is_aligned((uint64_t)area, FPU_ALIGN), "fpu: zone alignée");
	bytes = area;
	memset(bytes, 0, g_fpu.size);
	*(uint16_t *)bytes = FPU_FCW;
	*(uint32_t *)(bytes + FPU_MXCSR_OFF) = FPU_MXCSR;
	if (g_fpu.xsave)
		*(uint64_t *)(bytes + FPU_XSTATE_OFF) = XCR0_X87 | XCR0_SSE;
}

void	cpu_fpu_save(void *area)
{
	kassert_check(is_aligned((uint64_t)area, FPU_ALIGN), "fpu: zone alignée");
	if (g_fpu.xsave)
		fpu_xsave(area);
	else
		fpu_fxsave(area);
}

void	cpu_fpu_restore(const void *area)
{
	kassert_check(is_aligned((uint64_t)area, FPU_ALIGN), "fpu: zone alignée");
	if (g_fpu.xsave)
		fpu_xrstor(area);
	else
		fpu_fxrstor(area);
}
