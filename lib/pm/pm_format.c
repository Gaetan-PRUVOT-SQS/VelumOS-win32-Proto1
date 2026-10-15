#include "pm_int.h"

static int	put(t_text out, size_t *at, const char *s, size_t n)
{
	if (n >= out.cap - *at)
		return (E_OVERFLOW);
	memcpy(out.p + *at, s, n);
	*at += n;
	return (0);
}

static int	put_u32(t_text out, size_t *at, uint32_t v)
{
	char	d[10];
	size_t	i;

	i = 10;
	while (i == 10 || v)
	{
		i--;
		d[i] = (char)('0' + v % 10);
		v /= 10;
	}
	return (put(out, at, d + i, 10 - i));
}

static int	put_hex(t_text out, size_t *at, const uint8_t *cert)
{
	char	h[2 * PM_CERT_LEN];
	size_t	i;

	i = 0;
	while (i < PM_CERT_LEN)
	{
		h[2 * i] = "0123456789abcdef"[cert[i] >> 4];
		h[2 * i + 1] = "0123456789abcdef"[cert[i] & 15];
		i++;
	}
	return (put(out, at, h, sizeof(h)));
}

static int	put_entry(t_text out, size_t *at, const t_pminfo *i)
{
	if (!pm_info_ok(i))
		return (E_INVAL);
	if (put(out, at, i->package, pm_nlen(i->package, PM_PKG_MAX)) < 0
		|| put(out, at, ";", 1) < 0 || put_u32(out, at, i->version_code) < 0
		|| put(out, at, ";", 1) < 0
		|| put(out, at, i->label, pm_nlen(i->label, PM_LABEL_MAX)) < 0
		|| put(out, at, ";", 1) < 0
		|| put(out, at, i->activity, pm_nlen(i->activity, PM_ACT_MAX)) < 0
		|| put(out, at, ";", 1) < 0 || put_hex(out, at, i->cert) < 0
		|| put(out, at, "\n", 1) < 0)
		return (E_OVERFLOW);
	return (0);
}

int	pm_registry_format(const t_pmentry *e, uint32_t n, t_text out)
{
	size_t		at;
	uint32_t	i;
	int			r;

	if (!out.p || out.cap == 0 || (!e && n) || n > PM_MAX_PACKAGES)
		return (E_INVAL);
	at = 0;
	i = 0;
	while (i < n)
	{
		r = put_entry(out, &at, &e[i].info);
		if (r < 0)
			return (r);
		i++;
	}
	out.p[at] = '\0';
	return ((int)at);
}
