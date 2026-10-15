#include "ctl_int.h"

static uint32_t	take(const t_ctl *c, const t_edit *e, const char *s, uint32_t n)
{
	uint32_t	len;
	uint32_t	room;
	uint32_t	chars;
	uint32_t	pos;
	uint32_t	l;

	len = (uint32_t)strlen(c->text);
	room = CTL_TEXT_MAX - 1 - len;
	chars = ctl_u8_count(c->text, len);
	pos = 0;
	while (pos < n && chars < e->max_chars)
	{
		l = ctl_u8_len(s + pos, n - pos);
		if (pos + l > room)
			break ;
		pos += l;
		chars++;
	}
	return (pos);
}

bool	ctl_edt_insert(t_ctl *c, t_edit *e, const char *s, uint32_t n)
{
	bool		changed;
	uint32_t	len;
	uint32_t	k;

	changed = ctl_edt_erase_sel(c, e);
	k = take(c, e, s, n);
	if (k == 0)
		return (changed);
	len = (uint32_t)strlen(c->text);
	memmove(c->text + e->caret + k, c->text + e->caret, len - e->caret + 1);
	memcpy(c->text + e->caret, s, k);
	e->caret += k;
	e->anchor = e->caret;
	return (true);
}

bool	ctl_edt_erase_prev(t_ctl *c, t_edit *e)
{
	uint32_t	from;
	uint32_t	len;

	if (ctl_edt_erase_sel(c, e))
		return (true);
	if (e->caret == 0)
		return (false);
	len = (uint32_t)strlen(c->text);
	from = ctl_u8_prev(c->text, e->caret);
	memmove(c->text + from, c->text + e->caret, len - e->caret + 1);
	e->caret = from;
	e->anchor = from;
	return (true);
}

bool	ctl_edt_erase_next(t_ctl *c, t_edit *e)
{
	uint32_t	to;
	uint32_t	len;

	if (ctl_edt_erase_sel(c, e))
		return (true);
	len = (uint32_t)strlen(c->text);
	if (e->caret >= len)
		return (false);
	to = ctl_u8_next(c->text, len, e->caret);
	memmove(c->text + e->caret, c->text + to, len - to + 1);
	e->anchor = e->caret;
	return (true);
}
