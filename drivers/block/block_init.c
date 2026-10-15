#include "vblk.h"

static void	legacy_report(void)
{
	const t_pcidev	*d;
	uint32_t		i;

	i = 0;
	d = pci_find(VIO_VENDOR, VBLK_PCI_LEGACY, i);
	while (d)
	{
		klog_warn("block: virtio-blk legacy %02x:%02x.%u ignoré "
			"(VERSION_1 exigé)", d->bus, d->dev, d->fn);
		i++;
		d = pci_find(VIO_VENDOR, VBLK_PCI_LEGACY, i);
	}
}

static uint32_t	vblk_scan(void)
{
	const t_pcidev	*d;
	uint32_t		i;
	uint32_t		disks;

	i = 0;
	disks = 0;
	d = pci_find(VIO_VENDOR, VBLK_PCI_DEVICE, i);
	while (d && disks < VBLK_MAX_DISKS)
	{
		if (vblk_probe(d, disks) == 0)
			disks++;
		i++;
		d = pci_find(VIO_VENDOR, VBLK_PCI_DEVICE, i);
	}
	if (d)
		klog_warn("block: plus de %u disques virtio, suivants ignorés",
			VBLK_MAX_DISKS);
	return (disks);
}

static void	scan_all_partitions(void)
{
	uint32_t	n;
	uint32_t	i;
	t_blkdev	*d;
	int			rc;

	n = blk_count();
	i = 0;
	while (i < n)
	{
		d = blk_at(i);
		if (d && !d->parent)
		{
			rc = blk_scan_partitions(d);
			if (rc < 0)
				klog_warn("block: %s: table de partitions refusée (%d)",
					d->name, rc);
		}
		i++;
	}
}

int	block_boot_init(void)
{
	blk_reg();
	legacy_report();
	if (vblk_scan() == 0)
		klog_info("block: aucun disque virtio");
	scan_all_partitions();
	return (0);
}
