#include "fake_fab.h"

void	fab_q35(void)
{
	static const t_fabspec	host = {0, 0, 0, 0x8086, 0x29c0, 6, 0, 0};
	static const t_fabspec	vga = {0, 1, 0, 0x1234, 0x1111, 3, 0, 0};
	static const t_fabspec	isa = {0, 31, 0, 0x8086, 0x2918, 6, 1, 0x80};
	t_fabdev				*d;

	fab_make(&host);
	d = fab_make(&vga);
	fab_bar(d, 0, FAB_MEM32, 0x1000000);
	fab_bar_base(d, 0, 0xfd000000u);
	d->bar[0].pref = 1;
	fab_bar(d, 2, FAB_MEM32, 0x1000);
	fab_bar_base(d, 2, 0xfebf0000u);
	fab_irq(d, 10, 1);
	fab_make(&isa);
}
