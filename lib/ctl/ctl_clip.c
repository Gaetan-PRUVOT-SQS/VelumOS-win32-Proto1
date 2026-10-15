#include "ctl_int.h"

static t_clip	g_ctl_clip;

int	ctl_clip_set(const char *utf8, uint32_t len)
{
	if (!utf8 && len)
		return (E_INVAL);
	if (len > CTL_TEXT_MAX - 1)
		return (E_RANGE);
	if (len)
		memcpy(g_ctl_clip.data, utf8, len);
	g_ctl_clip.data[len] = '\0';
	g_ctl_clip.len = len;
	return (0);
}

int	ctl_clip_get(char *out, uint32_t cap)
{
	uint32_t	n;
	uint32_t	next;

	if (!out || cap == 0)
		return (E_INVAL);
	n = 0;
	next = ctl_u8_next(g_ctl_clip.data, g_ctl_clip.len, 0);
	while (n < g_ctl_clip.len && next <= cap - 1)
	{
		n = next;
		next = ctl_u8_next(g_ctl_clip.data, g_ctl_clip.len, n);
	}
	if (n)
		memcpy(out, g_ctl_clip.data, n);
	out[n] = '\0';
	return ((int)n);
}

void	ctl_clip_clear(void)
{
	g_ctl_clip.data[0] = '\0';
	g_ctl_clip.len = 0;
}
