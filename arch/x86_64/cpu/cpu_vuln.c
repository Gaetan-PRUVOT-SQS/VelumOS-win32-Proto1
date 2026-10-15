#include "velum/boot.h"
#include "velum/klog.h"
#include "cpu_vuln.h"

static void	audit_option(void)
{
	if (!boot_cmdline_has("entryaudit"))
		return ;
	if (!CPU_FAULT_INJECT)
	{
		klog_warn("cpu: option entryaudit ignorée (profil release)");
		return ;
	}
	cpu_entry_audit_enable();
}

void	cpu_hard_boot(void)
{
	audit_option();
}
