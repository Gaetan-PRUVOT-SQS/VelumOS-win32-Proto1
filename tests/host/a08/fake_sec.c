#include "fake.h"

int64_t	fk_map(int64_t h, uint64_t hint, uint64_t prot, uint64_t off)
{
	uint64_t	a[6];

	a[0] = (uint64_t)h;
	a[1] = hint;
	a[2] = prot;
	a[3] = off;
	a[4] = 2 * PAGE_SIZE;
	a[5] = 0;
	if (off == 1)
		a[4] = PAGE_SIZE + 1;
	return (fk_call6(SYS_SECTION_MAP, a));
}
