#include "ctl_int.h"

uint32_t	ctl_edt_dpos(const t_ctl *c, uint32_t off)
{
	if (c->flags & CTL_PASSWORD)
		return (ctl_u8_count(c->text, off));
	return (off);
}

bool	ctl_edt_view(const t_ctl *c, t_edview *v)
{
	uint32_t	n;

	v->font = ctl_font_of(c);
	if (!v->font)
		return (false);
	if (c->flags & CTL_PASSWORD)
	{
		n = ctl_u8_count(c->text, (uint32_t)strlen(c->text));
		memset(v->disp, '*', n);
		v->disp[n] = '\0';
		v->len = (int32_t)n;
		return (true);
	}
	ctl_copy_text(v->disp, c->text);
	v->len = (int32_t)strlen(v->disp);
	return (true);
}

int32_t	ctl_edt_wv(const t_edview *v, const t_ctl *c, uint32_t upto)
{
	return (font_text_width(v->font, v->disp, (int32_t)ctl_edt_dpos(c, upto)));
}

int32_t	ctl_edt_width(const t_ctl *c, uint32_t upto)
{
	t_edview	v;

	if (!ctl_edt_view(c, &v))
		return (0);
	return (ctl_edt_wv(&v, c, upto));
}

uint32_t	ctl_edt_index_at(const t_ctl *c, int32_t x)
{
	t_edview	v;
	uint32_t	len;
	uint32_t	pos;
	uint32_t	next;
	int32_t		w[2];

	len = (uint32_t)strlen(c->text);
	if (!ctl_edt_view(c, &v))
		return (0);
	pos = 0;
	w[0] = 0;
	while (pos < len)
	{
		next = ctl_u8_next(c->text, len, pos);
		w[1] = ctl_edt_wv(&v, c, next);
		if (x < (w[0] + w[1]) / 2)
			return (pos);
		pos = next;
		w[0] = w[1];
	}
	return (len);
}
