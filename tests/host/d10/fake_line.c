#include <stdio.h>
#include "fake.h"

const char	*fake_hex(void)
{
	return ("0123456789abcDEF0123456789abcDEF"
		"0123456789abcdef0123456789abcdef");
}

int	fake_ok(const char *p, const char *v, const char *l, const char *a)
{
	char		line[1024];
	t_pmentry	e[2];
	t_span		s;
	int			n;

	n = snprintf(line, sizeof(line), "%s;%s;%s;%s;%s\n", p, v, l, a,
			fake_hex());
	s.p = (const uint8_t *)line;
	s.len = (size_t)n;
	return (pm_registry_parse(s, e, 2) == 1);
}

char	*fake_rep(char *buf, size_t n, const char *tail)
{
	memset(buf, 'a', n);
	strlcpy(buf + n, tail, 8);
	return (buf);
}

int	fake_base(char *dst, size_t cap)
{
	return (snprintf(dst, cap, "com.a;1;Bonjour;Lcom/a/Main;;%s\n"
			"org.b_2.c;4294967295;Caf\xc3\xa9 \xf0\x9f\x98\x80;Lo/B$1;;%s\r\n"
			"\nx.y;0;;Lx/Y;;%s\n", fake_hex(), fake_hex(), fake_hex()));
}
