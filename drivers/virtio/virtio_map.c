#include "velum/err.h"
#include "velum/libk.h"
#include "virtio.h"

static volatile uint8_t	*cap_base(t_vio *v, int type, volatile uint8_t **bars)
{
	const t_viocap	*c;

	c = &v->caps[type];
	if (!c->found)
		return (NULL);
	if (!bars[c->bar])
		bars[c->bar] = (volatile uint8_t *)pci_map_bar(v->pci, c->bar);
	if (!bars[c->bar])
		return (NULL);
	return (bars[c->bar] + c->offset);
}

int	vio_map(t_vio *v)
{
	volatile uint8_t	*bars[PCI_BARS];

	memset(bars, 0, sizeof(bars));
	v->common = cap_base(v, VIO_CAP_COMMON, bars);
	v->notify = cap_base(v, VIO_CAP_NOTIFY, bars);
	v->isr = cap_base(v, VIO_CAP_ISR, bars);
	v->devcfg = cap_base(v, VIO_CAP_DEVICE, bars);
	if (!v->common || !v->notify)
		return (E_NOMEM);
	if (v->caps[VIO_CAP_DEVICE].found && !v->devcfg)
		return (E_NOMEM);
	v->notify_len = v->caps[VIO_CAP_NOTIFY].length;
	v->notify_mult = v->caps[VIO_CAP_NOTIFY].mult;
	v->devcfg_len = 0;
	if (v->devcfg)
		v->devcfg_len = v->caps[VIO_CAP_DEVICE].length;
	return (0);
}
