#include "zip_int.h"

int	inf_bits(t_inf *s, uint32_t n, uint32_t *v)
{
	while (s->nb < n)
	{
		if (s->pos >= s->in.len)
			return (E_INVAL);
		s->acc |= (uint32_t)s->in.p[s->pos] << s->nb;
		s->pos++;
		s->nb += 8;
	}
	*v = s->acc & ((1u << n) - 1);
	s->acc >>= n;
	s->nb -= n;
	return (0);
}

static void	inf_take(t_inf *s, uint32_t len)
{
	uint32_t	i;

	i = 0;
	while (i < len)
	{
		s->out[s->n + i] = s->in.p[s->pos + i];
		i++;
	}
	s->n += len;
	s->pos += len;
}

int	inf_stored(t_inf *s)
{
	uint32_t	len;

	s->pos -= s->nb / 8;
	s->acc = 0;
	s->nb = 0;
	if (s->in.len - s->pos < 4)
		return (E_INVAL);
	len = zip_rd16(s->in, s->pos);
	if ((len ^ zip_rd16(s->in, s->pos + 2)) != 0xffff)
		return (E_INVAL);
	s->pos += 4;
	if (s->in.len - s->pos < len)
		return (E_INVAL);
	if (s->cap - s->n < len)
		return (E_OVERFLOW);
	inf_take(s, len);
	return (0);
}
