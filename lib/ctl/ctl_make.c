#include "ctl_int.h"

static bool	make_ok(const t_ctlspec *spec, const t_ctlops *ops)
{
	if (!ops || !ops->supported)
		return (false);
	return (ctl_rect_ok(spec->rect));
}

t_ctl	*ctl_make(const t_ctlspec *spec, size_t extra)
{
	const t_ctlops	*ops;
	t_ctl			*c;
	size_t			priv;

	ops = ctl_ops_get(spec->type);
	if (!make_ok(spec, ops))
		return (NULL);
	priv = ops->priv_size + extra;
	c = ctl_alloc(sizeof(t_ctl) + priv);
	if (!c)
		return (NULL);
	c->type = spec->type;
	c->id = spec->id;
	c->flags = spec->flags & CTL_FLAGS_ALL;
	c->rect = spec->rect;
	if (priv)
		c->priv = (char *)c + sizeof(t_ctl);
	ctl_copy_text(c->text, spec->text);
	if (ops->init)
		ops->init(c);
	return (c);
}
