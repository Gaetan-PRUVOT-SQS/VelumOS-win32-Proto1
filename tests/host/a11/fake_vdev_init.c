#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "virtio.h"

t_fkdev	g_fkdev[FK_DEVS];

void	fk_dev_cfg32(t_fkdev *f, uint32_t off, uint32_t v)
{
	fk_le32(f->cfg + off, v);
}

static void	fk_cap(t_fkdev *f, uint32_t pos, uint32_t hdr, uint32_t off)
{
	fk_dev_cfg32(f, pos, hdr);
	fk_dev_cfg32(f, pos + 4, FK_BAR);
	fk_dev_cfg32(f, pos + 8, off);
	fk_dev_cfg32(f, pos + 12, FK_REGION);
}

static void	fk_caps(t_fkdev *f)
{
	fk_dev_cfg32(f, 0x00, VIO_VENDOR | ((uint32_t)f->pci.device << 16));
	fk_dev_cfg32(f, 0x04, 0x10u << 16);
	fk_dev_cfg32(f, 0x34, 0x40);
	fk_dev_cfg32(f, 0x40, 0x5011);
	fk_cap(f, 0x50, 0x01106009, FK_COMMON);
	fk_cap(f, 0x60, 0x03107009, FK_ISR);
	fk_cap(f, 0x70, 0x04108009, FK_DEVCFG);
	fk_cap(f, 0x80, 0x02140009, FK_NOTIFY);
	fk_dev_cfg32(f, 0x90, FK_MULT);
}

void	fk_dev_init(t_fkdev *f, uint64_t sectors, uint16_t device)
{
	memset(f, 0, sizeof(*f));
	f->present = true;
	f->pci.vendor = VIO_VENDOR;
	f->pci.device = device;
	f->pci.dev = (uint8_t)(f - g_fkdev + 2);
	f->pci.bar_flags[FK_BAR] = PCI_BAR_MEM64;
	f->pci.bar_size[FK_BAR] = FK_BAR_SIZE;
	f->bar = aligned_alloc(4096, FK_BAR_SIZE);
	f->disk_bytes = sectors * 512;
	f->disk = calloc(sectors + 1, 512);
	if (!f->bar || !f->disk)
		abort();
	memset(f->bar, 0, FK_BAR_SIZE);
	f->features = VIO_F_VERSION_1 | 0x200;
	f->qmax = 256;
	f->qsize = 256;
	fk_le64(f->devcfg, sectors);
	fk_le32(f->devcfg + 20, 512);
	fk_caps(f);
}

void	fk_dev_free(t_fkdev *f)
{
	free(f->bar);
	free(f->disk);
	memset(f, 0, sizeof(*f));
}
