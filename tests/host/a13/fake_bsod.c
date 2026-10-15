#include "kfix.h"

static const char	*g_pieces[] = {"a", "b", " ", "\n", "\xc3\xa9",
	"\xe2\x82\xac", "\xf0\x9f\x98\x80", "mot", "  ", "x"};

void	fake_text_gen(char *buf, size_t size, uint64_t *seed)
{
	size_t	n;
	size_t	len;
	size_t	pick;

	n = 0;
	len = kfix_rnd(seed) % 40;
	while (len--)
	{
		pick = kfix_rnd(seed) % (sizeof(g_pieces) / sizeof(g_pieces[0]));
		if (n + strlen(g_pieces[pick]) + 1 >= size)
			break ;
		memcpy(buf + n, g_pieces[pick], strlen(g_pieces[pick]));
		n += strlen(g_pieces[pick]);
	}
	buf[n] = '\0';
}
