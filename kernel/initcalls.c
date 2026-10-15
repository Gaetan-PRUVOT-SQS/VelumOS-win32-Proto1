#include "steps.h"
#include "velum/display.h"
#include "velum/err.h"
#include "velum/initcalls.h"
#include "velum/klog.h"
#include "velum/panic.h"

static const t_step	g_steps[] = {
{"cpu", cpu_boot_init}, {"pmm", pmm_boot_init}, {"vmm", vmm_boot_init},
{"heap", heap_boot_init}, {"display", display_boot_init},
{"acpi", acpi_boot_init}, {"irq", irq_boot_init}, {"timer", timer_boot_init},
{"sched", sched_boot_init}, {"syscall", syscall_boot_init},
{"object", object_boot_init}, {"proc", proc_boot_init},
{"random", random_boot_init}, {"pci", pci_boot_init},
{"input", input_boot_init}, {"block", block_boot_init},
{"vfs", vfs_boot_init}, {NULL, NULL}
};

static void	step_run(const t_step *s, uint32_t idx, uint32_t total)
{
	int	rc;

	if (!s->fn)
	{
		klog_info("boot: %s absent", s->name);
		return ;
	}
	rc = s->fn();
	if (rc < 0)
		panic("boot: %s a échoué (%d)", s->name, rc);
	klog_info("boot: %s ok", s->name);
	display_boot_progress((idx + 1) * 100 / total, s->name);
}

void	initcalls_run(void)
{
	uint32_t	total;
	uint32_t	i;

	total = 0;
	while (g_steps[total].name)
		total++;
	i = 0;
	while (i < total)
	{
		step_run(&g_steps[i], i, total);
		i++;
	}
}
