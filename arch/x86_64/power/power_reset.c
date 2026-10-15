#include "../../../kernel/time/time_int.h"
#include "velum/acpi.h"
#include "velum/io.h"
#include "velum/irqflags.h"
#include "velum/klog.h"

#define KBC_STATUS 0x64
#define KBC_CMD 0x64
#define KBC_IBF 0x02
#define KBC_RESET 0xfe
#define RCR_PORT 0xcf9
#define RCR_SYS_RST 0x02
#define RCR_RST_CPU 0x04
#define RESET_SETTLE_NS 100000000ull
#define RCR_GAP_NS 50000ull
#define KBC_TIMEOUT_NS 10000000ull

static bool	kbc_ready(void *ctx)
{
	(void)ctx;
	return (!(inb(KBC_STATUS) & KBC_IBF));
}

static void	reset_acpi(const t_acpi_info *i)
{
	if (!i->reset_port)
		return ;
	klog_info("power: RESET_REG port %#x valeur %#x", i->reset_port,
		i->reset_value);
	outb(i->reset_port, i->reset_value);
	time_delay_ns(RESET_SETTLE_NS);
}

static void	reset_cf9(void)
{
	klog_info("power: registre de reset %#x", RCR_PORT);
	outb(RCR_PORT, RCR_SYS_RST);
	time_delay_ns(RCR_GAP_NS);
	outb(RCR_PORT, RCR_SYS_RST | RCR_RST_CPU);
	time_delay_ns(RESET_SETTLE_NS);
}

static void	reset_kbc(void)
{
	if (wait_until(kbc_ready, NULL, KBC_TIMEOUT_NS) < 0)
		return ;
	klog_info("power: impulsion de reset par le contrôleur clavier");
	outb(KBC_CMD, KBC_RESET);
	time_delay_ns(RESET_SETTLE_NS);
}

int	power_reboot(void)
{
	klog_info("power: redémarrage");
	irq_disable();
	reset_acpi(acpi_info());
	reset_cf9();
	reset_kbc();
	klog_err("power: reset matériel sans effet, triple faute volontaire");
	power_triple_fault();
}
