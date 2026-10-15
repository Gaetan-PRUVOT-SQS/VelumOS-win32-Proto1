#include "steps.h"
#include "velum/initcalls.h"
#include "velum/klog.h"

static const t_step	g_tests[] = {
{"cpu", cpu_selftest}, {"pmm", pmm_selftest}, {"vmm", vmm_selftest},
{"heap", heap_selftest}, {"display", display_selftest},
{"acpi", acpi_selftest}, {"irq", irq_selftest}, {"timer", timer_selftest},
{"sched", sched_selftest}, {"syscall", syscall_selftest},
{"object", object_selftest}, {"proc", proc_selftest},
{"random", random_selftest}, {"pci", pci_selftest},
{"input", input_selftest}, {"block", block_selftest},
{"vfs", vfs_selftest}, {NULL, NULL}
};

static int	test_run(const t_step *t)
{
	int	rc;

	kprintf("[TEST] %s ...\n", t->name);
	rc = t->fn();
	if (rc)
		kprintf("[TEST] %s ... FAIL (%d)\n", t->name, rc);
	else
		kprintf("[TEST] %s ... OK\n", t->name);
	return (rc != 0);
}

int	selftests_run(void)
{
	int	i;
	int	run;
	int	fails;

	i = 0;
	run = 0;
	fails = 0;
	while (g_tests[i].name)
	{
		if (g_tests[i].fn)
		{
			fails += test_run(&g_tests[i]);
			run++;
		}
		i++;
	}
	if (fails)
		kprintf("SELFTESTS FAIL %d/%d\n", fails, run);
	else
		kprintf("SELFTESTS PASS %d\n", run);
	return (fails);
}
