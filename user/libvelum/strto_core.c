#include "ctype.h"
#include "strto_int.h"
#include "velum/err.h"

static int	strto_digit(int c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'z')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'Z')
		return (c - 'A' + 10);
	return (-1);
}

static int	strto_has_hex_prefix(const char *p)
{
	int	d;

	if (p[0] != '0' || (p[1] != 'x' && p[1] != 'X'))
		return (0);
	d = strto_digit((unsigned char)p[2]);
	return (d >= 0 && d < 16);
}

static const char	*strto_prefix(const char *p, int *base)
{
	if ((*base == 0 || *base == 16) && strto_has_hex_prefix(p))
	{
		*base = 16;
		return (p + 2);
	}
	if (*base == 0 && p[0] == '0')
		*base = 8;
	else if (*base == 0)
		*base = 10;
	return (p);
}

static void	strto_accumulate(const char *p, int base, t_strto *r)
{
	int	d;

	d = strto_digit((unsigned char)*p);
	while (d >= 0 && d < base)
	{
		if (r->mag > (UINT64_MAX - (uint64_t)d) / (uint64_t)base)
			r->overflow = 1;
		else
			r->mag = r->mag * (uint64_t)base + (uint64_t)d;
		p++;
		r->end = p;
		d = strto_digit((unsigned char)*p);
	}
}

void	strto_parse(const char *s, int base, t_strto *r)
{
	const char	*p;

	r->mag = 0;
	r->end = s;
	r->neg = 0;
	r->overflow = 0;
	r->status = 0;
	if (base < 0 || base == 1 || base > 36)
	{
		r->status = E_INVAL;
		return ;
	}
	p = s;
	while (isspace((unsigned char)*p))
		p++;
	if (*p == '+' || *p == '-')
	{
		r->neg = (*p == '-');
		p++;
	}
	p = strto_prefix(p, &base);
	strto_accumulate(p, base, r);
}
