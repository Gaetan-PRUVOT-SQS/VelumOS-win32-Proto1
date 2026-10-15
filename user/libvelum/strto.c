#include <limits.h>
#include "errno.h"
#include "stdlib.h"
#include "strto_int.h"

static long long	strto_signed(const t_strto *r)
{
	if (r->status)
	{
		errno = EINVAL;
		return (0);
	}
	if (r->neg && (r->overflow || r->mag > (uint64_t)LLONG_MAX + 1))
	{
		errno = ERANGE;
		return (LLONG_MIN);
	}
	if (!r->neg && (r->overflow || r->mag > (uint64_t)LLONG_MAX))
	{
		errno = ERANGE;
		return (LLONG_MAX);
	}
	if (r->neg)
		return ((long long)(0 - r->mag));
	return ((long long)r->mag);
}

long long	strtoll(const char *s, char **end, int base)
{
	t_strto	r;

	strto_parse(s, base, &r);
	if (end)
		*end = (char *)r.end;
	return (strto_signed(&r));
}

long	strtol(const char *s, char **end, int base)
{
	return ((long)strtoll(s, end, base));
}

unsigned long long	strtoull(const char *s, char **end, int base)
{
	t_strto	r;

	strto_parse(s, base, &r);
	if (end)
		*end = (char *)r.end;
	if (r.status)
	{
		errno = EINVAL;
		return (0);
	}
	if (r.overflow)
	{
		errno = ERANGE;
		return (ULLONG_MAX);
	}
	if (r.neg)
		return (0 - r.mag);
	return (r.mag);
}

unsigned long	strtoul(const char *s, char **end, int base)
{
	return ((unsigned long)strtoull(s, end, base));
}
