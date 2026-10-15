#include "ctl_int.h"

static bool	radio_ok(const t_ctl *s)
{
	return (s->type == CT_RADIO && (s->flags & CTL_VISIBLE)
		&& (s->flags & CTL_ENABLED));
}

static void	radio_scan(const t_ctl *c, t_focusscan *sc)
{
	t_ctl	*s;
	bool	past;

	memset(sc, 0, sizeof(*sc));
	past = false;
	s = c->parent->first;
	while (s)
	{
		if (s == c)
			past = true;
		if (radio_ok(s))
		{
			if (!sc->first)
				sc->first = s;
			sc->last = s;
			if (s != c && !past)
				sc->before = s;
			else if (s != c && !sc->after)
				sc->after = s;
		}
		s = s->next;
	}
}

void	ctl_radio_select(t_ctlroot *r, t_ctl *c)
{
	t_ctl	*sib;
	bool	has;

	sib = c->parent->first;
	while (sib)
	{
		has = (sib->flags & CTL_CHECKED) != 0;
		if (sib->type == CT_RADIO && has != (sib == c))
		{
			sib->flags ^= CTL_CHECKED;
			ctl_dirty_add(r, sib->rect);
		}
		sib = sib->next;
	}
}

bool	ctl_radio_stop(const t_ctl *c)
{
	const t_ctl	*sib;
	const t_ctl	*first;

	first = NULL;
	sib = c->parent->first;
	while (sib)
	{
		if (radio_ok(sib) && (sib->flags & CTL_CHECKED))
			return (sib == c);
		if (radio_ok(sib) && !first)
			first = sib;
		sib = sib->next;
	}
	return (first == c);
}

t_ctl	*ctl_radio_step(const t_ctl *c, bool forward)
{
	t_focusscan	sc;

	radio_scan(c, &sc);
	if (forward && sc.after)
		return (sc.after);
	if (forward)
		return (sc.first);
	if (sc.before)
		return (sc.before);
	return (sc.last);
}
