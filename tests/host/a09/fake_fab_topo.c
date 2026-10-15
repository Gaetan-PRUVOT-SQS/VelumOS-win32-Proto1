#include "fake_fab.h"

void	fab_endpoint(uint8_t bus, uint8_t dev, uint16_t vendor)
{
	t_fabspec	s;

	s = (t_fabspec){bus, dev, 0, vendor, 0x0001, 2, 0, 0};
	fab_make(&s);
}

void	fab_pci_bridge(uint8_t bus, uint8_t dev, uint8_t sec, uint8_t sub)
{
	t_fabspec	s;

	s = (t_fabspec){bus, dev, 0, 0x8086, 0x244e, 6, 4, 1};
	fab_bridge(fab_make(&s), sec, sub);
}

void	fab_bridge_chain(int length)
{
	int	b;

	b = 0;
	while (b < length)
	{
		fab_pci_bridge((uint8_t)b, 0, (uint8_t)(b + 1), (uint8_t)length);
		fab_endpoint((uint8_t)b, 1, (uint16_t)(0x1000 + b));
		b++;
	}
}

void	fab_multifunction_slots(int slots)
{
	t_fabspec	s;
	int			dev;
	int			fn;

	dev = 0;
	while (dev < slots)
	{
		fn = 0;
		while (fn < 8)
		{
			s = (t_fabspec){0, (uint8_t)dev, (uint8_t)fn, 0x1234,
				(uint16_t)(dev * 8 + fn + 1), 2, 0, (uint8_t)(0x80 * !fn)};
			fab_make(&s);
			fn++;
		}
		dev++;
	}
}

t_fabdev	*fab_msi_dev(uint8_t dev, uint16_t ctrl)
{
	t_fabspec	s;
	t_fabdev	*d;

	s = (t_fabspec){0, dev, 0, 0xabcd, 0x0001, 2, 0, 0};
	d = fab_make(&s);
	fab_put(d, 0x34, 1, 0x50);
	fab_cap(d, 0x50, 0x05, 0);
	fab_put(d, 0x52, 2, ctrl);
	d->msi_off = 0x50;
	return (d);
}
