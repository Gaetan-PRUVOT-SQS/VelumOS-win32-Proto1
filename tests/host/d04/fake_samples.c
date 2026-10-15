#include "fake.h"

static const uint16_t	g_loop[6] = {0x5012, 0x00d8, 0xff00, 0x0039, 0xfffe,
	0x000e};
static const uint16_t	g_packed[14] = {0x002b, 6, 0, 0x000e, 0x000e, 0,
	0x0100, 2, 0, 0, 3, 0, 4, 0};
static const uint16_t	g_sparse[25] = {0x002c, 8, 0, 0x0126, 15, 0, 0x000e,
	0x000e, 0x0200, 2, 0xfffb, 0xffff, 10, 0, 6, 0, 7, 0, 0x0300, 2, 3, 0, 1,
	2, 3};
static const uint16_t	g_calls[12] = {0x0013, 0x7fff, 0x2071, 3, 0x0010,
	0x010a, 0x0274, 2, 0, 0x0116, 5, 0x000e};

uint32_t	fake_sample(int i)
{
	if (i == 0)
		fake_us(g_loop, 6);
	if (i == 1)
		fake_us(g_packed, 14);
	if (i == 2)
		fake_us(g_sparse, 25);
	if (i == 3)
		fake_us(g_calls, 12);
	return (3);
}
