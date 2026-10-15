#include "kfix.h"

int	kfix_chunked(t_kfix *f, const char *buf, size_t n)
{
	size_t	i;
	size_t	len;
	int		bad;

	i = 0;
	bad = 0;
	while (i < n)
	{
		len = 1 + (i * 7) % 97;
		if (len > n - i)
			len = n - i;
		kfix_feed_n(f, buf + i, len);
		bad += !kfix_ok(f);
		i += len;
	}
	return (bad);
}

int	kfix_fuzz_once(t_kfix *f, uint64_t *seed)
{
	char	buf[64];
	size_t	n;
	size_t	i;

	n = kfix_rnd(seed) % 41;
	i = 0;
	while (i < n)
	{
		buf[i] = (char)(kfix_rnd(seed) >> 24);
		i++;
	}
	kfix_feed_n(f, buf, n);
	return (!kfix_ok(f));
}
