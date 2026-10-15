#include "kfix.h"

bool	kfix_ok(const t_kfix *f)
{
	if (f->k.col < 0 || f->k.col > f->k.cols)
		return (false);
	return (f->k.row >= 0 && f->k.row < f->k.rows);
}

uint64_t	kfix_rnd(uint64_t *s)
{
	*s ^= *s << 13;
	*s ^= *s >> 7;
	*s ^= *s << 17;
	return (*s);
}

static char	stream_char(size_t i)
{
	if (i % 73 == 72)
		return ('\n');
	if (i % 11 == 10)
		return ('\t');
	if (i % 97 == 96)
		return ('\r');
	if (i % 53 == 52)
		return ('\b');
	return ((char)('a' + i % 26));
}

void	kfix_stream(char *buf, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		buf[i] = stream_char(i);
		i++;
	}
}
