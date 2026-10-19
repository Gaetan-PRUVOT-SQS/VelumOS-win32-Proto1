#include "velum/libk.h"
#include "velum/notif.h"

#define NS_S 1000000000ull
#define SEC_MIN 60ull
#define SEC_H 3600ull
#define SEC_D 86400ull

static size_t	put_str(char *out, size_t size, size_t at, const char *s)
{
	while (*s != '\0' && at + 1 < size)
		out[at++] = *s++;
	out[at] = '\0';
	return (at);
}

static size_t	put_num(char *out, size_t size, size_t at, uint64_t v)
{
	char	rev[21];
	size_t	n;

	n = 0;
	rev[n++] = (char)('0' + v % 10);
	v /= 10;
	while (v > 0)
	{
		rev[n++] = (char)('0' + v % 10);
		v /= 10;
	}
	while (n > 0 && at + 1 < size)
		out[at++] = rev[--n];
	out[at] = '\0';
	return (at);
}

size_t	notif_badge_text(uint32_t count, char *out, size_t size)
{
	size_t	at;

	if (out == NULL || size == 0)
		return (0);
	out[0] = '\0';
	if (count == 0)
		return (0);
	if (count > NOTIF_CAP_BADGE)
	{
		at = put_num(out, size, 0, NOTIF_CAP_BADGE);
		return (put_str(out, size, at, "+"));
	}
	return (put_num(out, size, 0, count));
}

static size_t	put_age(char *out, size_t size, uint64_t v, const char *unit)
{
	size_t	at;

	at = put_str(out, size, 0, "il y a ");
	at = put_num(out, size, at, v);
	at = put_str(out, size, at, " ");
	return (put_str(out, size, at, unit));
}

size_t	notif_age_text(uint64_t now_ns, uint64_t stamp_ns, char *out,
	size_t size)
{
	uint64_t	s;

	if (out == NULL || size == 0)
		return (0);
	out[0] = '\0';
	if (now_ns < stamp_ns || (now_ns - stamp_ns) / NS_S == 0)
		return (put_str(out, size, 0, "à l'instant"));
	s = (now_ns - stamp_ns) / NS_S;
	if (s < SEC_MIN)
		return (put_age(out, size, s, "s"));
	if (s < SEC_H)
		return (put_age(out, size, s / SEC_MIN, "min"));
	if (s < SEC_D)
		return (put_age(out, size, s / SEC_H, "h"));
	return (put_age(out, size, s / SEC_D, "j"));
}
