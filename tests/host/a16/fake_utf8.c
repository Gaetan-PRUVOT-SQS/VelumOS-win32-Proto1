#include "fake_utf8.h"

const t_utf8row	g_utf8_rows[UTF8_ROWS] = {
{0xC2, 0xDF, 0x80, 0xBF, 2},
{0xE0, 0xE0, 0xA0, 0xBF, 3},
{0xE1, 0xEC, 0x80, 0xBF, 3},
{0xED, 0xED, 0x80, 0x9F, 3},
{0xEE, 0xEF, 0x80, 0xBF, 3},
{0xF0, 0xF0, 0x90, 0xBF, 4},
{0xF1, 0xF3, 0x80, 0xBF, 4},
{0xF4, 0xF4, 0x80, 0x8F, 4},
};

static const t_utf8row	*find_row(uint8_t lead)
{
	size_t	i;

	i = 0;
	while (i < UTF8_ROWS)
	{
		if (lead >= g_utf8_rows[i].lead_lo && lead <= g_utf8_rows[i].lead_hi)
			return (&g_utf8_rows[i]);
		i++;
	}
	return (NULL);
}

static int	byte_ok(const t_utf8row *row, const uint8_t *p, size_t n)
{
	if (n == 1)
		return (p[1] >= row->second_lo && p[1] <= row->second_hi);
	return (p[n] >= 0x80 && p[n] <= 0xBF);
}

size_t	ref_utf8_decode(const uint8_t *p, size_t avail, uint32_t *cp)
{
	const t_utf8row	*row;
	size_t			n;

	*cp = p[0];
	if (p[0] < 0x80)
		return (1);
	*cp = UTF8_REPLACEMENT;
	row = find_row(p[0]);
	if (!row)
		return (1);
	n = 1;
	while (n < row->len && n < avail && byte_ok(row, p, n))
		n++;
	if (n < row->len)
		return (n);
	*cp = p[0] & (0x3F >> (row->len - 1));
	n = 1;
	while (n < row->len)
		*cp = (*cp << 6) | (p[n++] & 0x3F);
	return (row->len);
}

size_t	ref_utf8_encode(uint32_t cp, uint8_t *out)
{
	if (cp < 0x80)
		out[0] = (uint8_t)cp;
	if (cp < 0x80)
		return (1);
	if (cp < 0x800)
	{
		out[0] = 0xC0 | (cp >> 6);
		out[1] = 0x80 | (cp & 0x3F);
		return (2);
	}
	if (cp < 0x10000)
	{
		out[0] = 0xE0 | (cp >> 12);
		out[1] = 0x80 | ((cp >> 6) & 0x3F);
		out[2] = 0x80 | (cp & 0x3F);
		return (3);
	}
	out[0] = 0xF0 | (cp >> 18);
	out[1] = 0x80 | ((cp >> 12) & 0x3F);
	out[2] = 0x80 | ((cp >> 6) & 0x3F);
	out[3] = 0x80 | (cp & 0x3F);
	return (4);
}
