#include "ws_core.h"

static uint32_t	seq_len(uint8_t c)
{
	if (c < 0x80)
		return (1);
	if (c >= 0xc2 && c <= 0xdf)
		return (2);
	if (c >= 0xe0 && c <= 0xef)
		return (3);
	if (c >= 0xf0 && c <= 0xf4)
		return (4);
	return (0);
}

static bool	second_ok(uint8_t lead, uint8_t c)
{
	if (lead == 0xe0)
		return (c >= 0xa0 && c <= 0xbf);
	if (lead == 0xed)
		return (c >= 0x80 && c <= 0x9f);
	if (lead == 0xf0)
		return (c >= 0x90 && c <= 0xbf);
	if (lead == 0xf4)
		return (c >= 0x80 && c <= 0x8f);
	if (lead == 0xc2)
		return (c >= 0xa0 && c <= 0xbf);
	return (c >= 0x80 && c <= 0xbf);
}

bool	wsp_utf8_ok(const uint8_t *s, uint32_t n)
{
	uint32_t	i;
	uint32_t	k;
	uint32_t	j;

	i = 0;
	while (i < n)
	{
		k = seq_len(s[i]);
		if (k == 0 || k > n - i || s[i] < 0x20 || s[i] == 0x7f)
			return (false);
		if (k > 1 && !second_ok(s[i], s[i + 1]))
			return (false);
		j = 2;
		while (j < k)
		{
			if (s[i + j] < 0x80 || s[i + j] > 0xbf)
				return (false);
			j++;
		}
		i += k;
	}
	return (true);
}

bool	wsp_title_ok(const char *t, uint32_t max)
{
	uint32_t	n;

	n = 0;
	while (n < max && t[n] != '\0')
		n++;
	if (n == max)
		return (false);
	return (wsp_utf8_ok((const uint8_t *)t, n));
}

bool	wsp_rect_ok(t_rect r)
{
	return (r.x >= -WS_COORD_MAX && r.x <= WS_COORD_MAX
		&& r.y >= -WS_COORD_MAX && r.y <= WS_COORD_MAX
		&& r.w >= 1 && r.w <= WS_DIM_MAX && r.h >= 1 && r.h <= WS_DIM_MAX);
}
