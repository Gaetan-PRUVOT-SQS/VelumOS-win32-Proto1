#include "steps.h"
#include "velum/arch.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/initcalls.h"
#include "velum/klog.h"
#include "velum/panic.h"
#include "velum/proc.h"
#include "velum/sched.h"

#define VELUM_VERSION "0.1"

static void	boot_banner(void)
{
	const t_bootinfo	*bi;
	const char			*fw;

	bi = boot_info();
	fw = "BIOS";
	if (bi->efi)
		fw = "UEFI";
	klog_info("VelumOS %s, build %s", VELUM_VERSION, boot_build_id());
	klog_info("boot: %s, %u plages mémoire, hhdm %#llx", fw, bi->nranges,
		bi->hhdm);
	klog_info("boot: cmdline '%s'", bi->cmdline);
}

_Noreturn void	kmain(void)
{
	int	fails;

	serial_init();
	if (boot_collect() < 0)
		panic("boot: collecte des informations impossible");
	boot_banner();
	initcalls_run();
	klog_info("VelumOS boot ok");
	fails = 0;
	if (boot_cmdline_has("selftest"))
		fails = selftests_run();
	init_start();
	if (boot_cmdline_has("exit"))
		qemu_exit(fails != 0);
	sched_idle();
	arch_halt_forever();
}
