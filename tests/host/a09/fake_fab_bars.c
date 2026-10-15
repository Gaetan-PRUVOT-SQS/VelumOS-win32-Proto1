#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "pci_int.h"

void	*fab_bars_device(void)
{
	static const t_fabspec	spec = {0, 3, 0, 0x1234, 0x1111, 3, 0, 0};
	t_fabdev				*f;

	fab_reset();
	fake_reset();
	fake_mmio_reset();
	f = fab_make(&spec);
	fab_bar(f, 0, FAB_MEM32, 16);
	fab_bar_base(f, 0, 0xfebf1010u);
	fab_bar(f, 1, FAB_IO, 8);
	fab_bar_base(f, 1, 0xc040);
	fab_bar(f, 2, FAB_MEM32, 0x1000);
	fab_bar(f, 3, FAB_MEM64_LO, 0x20000000ull);
	fab_bar_base(f, 3, 0x2000000000ull);
	pci_scan_root(0, 0);
	return ((void *)pci_at(0));
}
