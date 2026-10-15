#include "velum/err.h"
#include "velum/libk.h"
#include "virtio.h"

#define PCI_STATUS_CAPS 0x10
#define PCI_CAP_PTR 0x34
#define PCI_CAP_FIRST 0x40
#define PCI_CFG_END 0x100

static bool	cap_usable(const t_pcidev *d, const t_viocap *c)
{
	uint64_t	size;

	if (c->bar >= PCI_BARS || (d->bar_flags[c->bar] & PCI_BAR_IO))
		return (false);
	size = d->bar_size[c->bar];
	if (c->length == 0 || c->offset > size || c->length > size - c->offset)
		return (false);
	return (true);
}

static void	cap_read(const t_pcidev *d, uint32_t pos, t_vio *v)
{
	uint32_t	w0;
	uint32_t	clen;
	uint32_t	type;
	t_viocap	*c;

	w0 = pci_cfg_read32(d, (uint16_t)pos);
	clen = (w0 >> 16) & 0xff;
	type = (w0 >> 24) & 0xff;
	if ((w0 & 0xff) != VIO_CAP_VNDR || clen < VIO_CAP_MIN_LEN
		|| pos + clen > PCI_CFG_END || type == 0 || type >= VIO_CAP_TYPES)
		return ;
	c = &v->caps[type];
	if (c->found || (type == VIO_CAP_NOTIFY && clen < VIO_NOTIFY_CAP_LEN))
		return ;
	c->bar = (uint8_t)(pci_cfg_read32(d, (uint16_t)(pos + 4)) & 0xff);
	c->offset = pci_cfg_read32(d, (uint16_t)(pos + 8));
	c->length = pci_cfg_read32(d, (uint16_t)(pos + 12));
	c->mult = 0;
	if (type == VIO_CAP_NOTIFY)
		c->mult = pci_cfg_read32(d, (uint16_t)(pos + 16));
	c->found = cap_usable(d, c);
}

static int	caps_complete(const t_vio *v)
{
	const t_viocap	*cc;
	const t_viocap	*nc;

	cc = &v->caps[VIO_CAP_COMMON];
	nc = &v->caps[VIO_CAP_NOTIFY];
	if (!cc->found || cc->length < VIO_COMMON_MIN)
		return (E_NODEV);
	if (!nc->found || nc->length < 2 || (nc->mult & (nc->mult - 1)))
		return (E_NODEV);
	return (0);
}

int	vio_find_caps(const t_pcidev *d, t_vio *v)
{
	uint32_t	pos;
	int			guard;

	memset(v, 0, sizeof(*v));
	v->pci = d;
	if (!((pci_cfg_read32(d, 0x04) >> 16) & PCI_STATUS_CAPS))
		return (E_NODEV);
	pos = pci_cfg_read32(d, PCI_CAP_PTR) & 0xfc;
	guard = 0;
	while (pos >= PCI_CAP_FIRST && pos < PCI_CFG_END && guard < VIO_CAPS_MAX)
	{
		cap_read(d, pos, v);
		pos = (pci_cfg_read32(d, (uint16_t)pos) >> 8) & 0xfc;
		guard++;
	}
	return (caps_complete(v));
}
