#include "ctl_int.h"

bool	ctl_edt_copy(t_ctl *c, t_edit *e)
{
	uint32_t	from;
	uint32_t	to;

	if (!ctl_edt_has_sel(e) || (c->flags & CTL_PASSWORD))
		return (false);
	from = ctl_edt_sel_from(e);
	to = ctl_edt_sel_to(e);
	return (ctl_clip_set(c->text + from, to - from) == 0);
}

static uint32_t	sanitize(const char *src, uint32_t n, char *dst)
{
	uint32_t	pos;
	uint32_t	out;
	uint32_t	l;

	pos = 0;
	out = 0;
	while (pos < n)
	{
		l = ctl_u8_len(src + pos, n - pos);
		if (l > 1 || ((uint8_t)src[pos] >= 0x20 && (uint8_t)src[pos] < 0x7f))
		{
			memcpy(dst + out, src + pos, l);
			out += l;
		}
		pos += l;
	}
	dst[out] = '\0';
	return (out);
}

bool	ctl_edt_paste(t_ctl *c, t_edit *e)
{
	char	clip[CTL_TEXT_MAX];
	char	clean[CTL_TEXT_MAX];
	int		n;

	n = ctl_clip_get(clip, sizeof(clip));
	if (n <= 0)
		return (false);
	n = (int)sanitize(clip, (uint32_t)n, clean);
	if (n == 0)
		return (false);
	return (ctl_edt_insert(c, e, clean, (uint32_t)n));
}
