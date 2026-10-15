#include "pci_int.h"
#include "velum/acpi.h"
#include "velum/klog.h"

int	pci_use_backends(const t_acpi_info *ai)
{
	t_pciops	ops;

	if (ai && ai->nmcfg > 0 && pci_ecam_init(ai->mcfg, ai->nmcfg) > 0)
	{
		pci_ecam_ops(&ops);
		pcicfg_set_ops(&ops);
		return (PCI_BACKEND_ECAM);
	}
	pci_legacy_ops(&ops);
	pcicfg_set_ops(&ops);
	return (PCI_BACKEND_LEGACY);
}

void	pci_scan_all(int backend)
{
	const t_ecam	*e;
	uint32_t		i;

	e = &pci_state()->ecam;
	if (backend != PCI_BACKEND_ECAM)
	{
		pci_scan_root(0, 0);
		return ;
	}
	i = 0;
	while (i < e->nwin)
	{
		pci_scan_root(e->win[i].segment, e->win[i].bus_start);
		i++;
	}
}

int	pci_boot_init(void)
{
	int	backend;

	pci_state_reset();
	backend = pci_use_backends(acpi_info());
	pci_scan_all(backend);
	if (!pci_state()->count && backend == PCI_BACKEND_ECAM)
	{
		klog_warn("pci: ECAM sans appareil, essai des ports 0xcf8/0xcfc");
		backend = pci_use_backends(NULL);
		pci_scan_all(backend);
	}
	pci_log_devices(backend);
	return (0);
}
