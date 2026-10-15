#include "zip_int.h"

static int	inf_block(t_inf *s, uint32_t type)
{
	int	r;

	if (type == 0)
		return (inf_stored(s));
	if (type == 3)
		return (E_INVAL);
	if (type == 1)
		r = inf_fixed(s);
	else
		r = inf_dynamic(s);
	if (r < 0)
		return (r);
	return (inf_codes(s));
}

int64_t	inflate_raw(t_span in, uint8_t *out, size_t cap)
{
	t_inf		s;
	uint32_t	last;
	uint32_t	type;
	int			r;

	s.in = in;
	s.pos = 0;
	s.acc = 0;
	s.nb = 0;
	s.out = out;
	s.cap = cap;
	s.n = 0;
	last = 0;
	while (last == 0)
	{
		if (inf_bits(&s, 1, &last) < 0 || inf_bits(&s, 2, &type) < 0)
			return (E_INVAL);
		r = inf_block(&s, type);
		if (r < 0)
			return (r);
	}
	return ((int64_t)s.n);
}
