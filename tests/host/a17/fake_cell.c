#include "fake_font.h"

static const uint32_t	g_acc[][3] = {{0xe9, 'e', 1}, {0xe8, 'e', 2},
{0xea, 'e', 3}, {0xeb, 'e', 4}, {0xe0, 'a', 2}, {0xe2, 'a', 3},
{0xf9, 'u', 2}, {0xfb, 'u', 3}, {0xf4, 'o', 3}, {0xee, 'i', 3},
{0xef, 'i', 4}, {0xe7, 'c', 5}, {0xc9, 'E', 1}, {0xc8, 'E', 2},
{0xc0, 'A', 2}, {0x153, 'o', 0}, {0xab, '<', 0}, {0xbb, '>', 0}, {0, 0, 0}};

static bool	fk_accent(int32_t acc, int32_t off, int32_t x, int32_t y)
{
	if (acc == 1)
		return ((y == off && x == 3) || (y == off + 1 && x == 2));
	if (acc == 2)
		return ((y == off && x == 1) || (y == off + 1 && x == 2));
	if (acc == 3)
		return ((y == off && x == 2) || (y == off + 1 && (x == 1 || x == 3)));
	if (acc == 4)
		return (y == off + 1 && (x == 1 || x == 3));
	return (acc == 5 && y == 9 && x == 2);
}

static bool	fk_base(int32_t ch, int32_t x, int32_t y)
{
	const char	*rows;

	rows = fk_rows(ch);
	if (rows == NULL)
		return ((x == 0 || x == 4 || y == 2 || y == 8) && y >= 2 && y <= 8);
	if (y < 2 || y > 8)
		return (false);
	return (rows[(y - 2) * 5 + x] == '#');
}

bool	fk_pixel(uint32_t cp, int32_t x, int32_t y)
{
	int32_t	i;
	int32_t	off;

	if (cp == 0x2026)
		return (y == 8 && (x == 0 || x == 2 || x == 4));
	i = 0;
	while (g_acc[i][0] != 0 && g_acc[i][0] != cp)
		i++;
	if (g_acc[i][0] == 0)
		return (fk_base((int32_t)cp, x, y));
	off = 2;
	if ((g_acc[i][1] >= 'A' && g_acc[i][1] <= 'Z') || g_acc[i][1] == 'i')
		off = 0;
	return (fk_base((int32_t)g_acc[i][1], x, y)
		|| fk_accent((int32_t)g_acc[i][2], off, x, y));
}
