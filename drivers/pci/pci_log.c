#include "pci_int.h"
#include "velum/klog.h"

static void	log_bars(const t_pcidev *d)
{
	uint32_t	i;

	i = 0;
	while (i < PCI_BARS)
	{
		if (d->bar_size[i])
			klog_debug("pci:   bar%u %s%s 0x%llx taille 0x%llx", i,
				pci_bar_kind(d->bar_flags[i]), pci_bar_pref(d->bar_flags[i]),
				(unsigned long long)d->bar[i],
				(unsigned long long)d->bar_size[i]);
		i++;
	}
}

static void	log_device(const t_pcidev *d)
{
	klog_info("pci: %04x:%02x:%02x.%u %04x:%04x %02x%02x%02x %s",
		d->seg, d->bus, d->dev, d->fn, d->vendor, d->device,
		d->class_code, d->subclass, d->prog_if,
		pci_class_name(d->class_code, d->subclass));
	log_bars(d);
}

void	pci_log_devices(int backend)
{
	const t_pcistate	*st;
	uint32_t			i;

	st = pci_state();
	klog_info("pci: %u appareil(s), configuration par %s", st->count,
		pci_backend_name(backend));
	if (st->overflow)
		klog_warn("pci: table pleine, %u appareil(s) ignore(s)", st->overflow);
	i = 0;
	while (i < st->count)
	{
		log_device(&st->devs[i]);
		i++;
	}
}
