#include <stdarg.h>
#include "velum/arch.h"
#include "velum/cpu.h"
#include "velum/irqflags.h"
#include "velum/klog.h"
#include "velum/panic.h"

extern uint64_t	cpu_read_cr2(void) __attribute__((weak));
extern uint64_t	cpu_read_cr3(void) __attribute__((weak));

static void	(*g_screen)(const char *t, const char *m);
static volatile int	g_in_panic;

void	panic_set_screen(void (*fn)(const char *t, const char *m))
{
	g_screen = fn;
}

static void	panic_report(const char *msg)
{
	kprintf("\nPANIC: %s\n", msg);
	if (g_screen)
		g_screen("VelumOS", msg);
	qemu_exit(0x31);
	arch_halt_forever();
}

_Noreturn void	panic(const char *fmt, ...)
{
	char	buf[192];
	va_list	ap;

	irq_disable();
	if (g_in_panic)
		arch_halt_forever();
	g_in_panic = 1;
	va_start(ap, fmt);
	kvsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	panic_report(buf);
	arch_halt_forever();
}

_Noreturn void	panic_regs(const struct s_regs *regs, const char *msg)
{
	kprintf("\nvec=%llu err=%#llx rip=%#llx rsp=%#llx cs=%#llx\n", regs->vec,
		regs->err, regs->rip, regs->rsp, regs->cs);
	kprintf("rax=%#llx rbx=%#llx rcx=%#llx rdx=%#llx\n", regs->rax,
		regs->rbx, regs->rcx, regs->rdx);
	if (cpu_read_cr2 && cpu_read_cr3)
		kprintf("cr2=%#llx cr3=%#llx\n", cpu_read_cr2(), cpu_read_cr3());
	panic("%s", msg);
}

void	kassert_check(int cond, const char *msg)
{
	if (cond)
		return ;
	panic("assertion: %s (from %p)", msg, __builtin_return_address(0));
}
