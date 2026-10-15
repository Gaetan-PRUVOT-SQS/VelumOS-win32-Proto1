#ifndef STEPS_H
# define STEPS_H

typedef struct s_step
{
	const char	*name;
	int			(*fn)(void);
}	t_step;

extern int	cpu_boot_init(void) __attribute__((weak));
extern int	pmm_boot_init(void) __attribute__((weak));
extern int	vmm_boot_init(void) __attribute__((weak));
extern int	heap_boot_init(void) __attribute__((weak));
extern int	display_boot_init(void) __attribute__((weak));
extern int	acpi_boot_init(void) __attribute__((weak));
extern int	irq_boot_init(void) __attribute__((weak));
extern int	timer_boot_init(void) __attribute__((weak));
extern int	sched_boot_init(void) __attribute__((weak));
extern int	syscall_boot_init(void) __attribute__((weak));
extern int	object_boot_init(void) __attribute__((weak));
extern int	proc_boot_init(void) __attribute__((weak));
extern int	random_boot_init(void) __attribute__((weak));
extern int	pci_boot_init(void) __attribute__((weak));
extern int	input_boot_init(void) __attribute__((weak));
extern int	block_boot_init(void) __attribute__((weak));
extern int	vfs_boot_init(void) __attribute__((weak));
extern int	cpu_selftest(void) __attribute__((weak));
extern int	pmm_selftest(void) __attribute__((weak));
extern int	vmm_selftest(void) __attribute__((weak));
extern int	heap_selftest(void) __attribute__((weak));
extern int	display_selftest(void) __attribute__((weak));
extern int	acpi_selftest(void) __attribute__((weak));
extern int	irq_selftest(void) __attribute__((weak));
extern int	timer_selftest(void) __attribute__((weak));
extern int	sched_selftest(void) __attribute__((weak));
extern int	syscall_selftest(void) __attribute__((weak));
extern int	object_selftest(void) __attribute__((weak));
extern int	proc_selftest(void) __attribute__((weak));
extern int	random_selftest(void) __attribute__((weak));
extern int	pci_selftest(void) __attribute__((weak));
extern int	input_selftest(void) __attribute__((weak));
extern int	block_selftest(void) __attribute__((weak));
extern int	vfs_selftest(void) __attribute__((weak));

#endif
