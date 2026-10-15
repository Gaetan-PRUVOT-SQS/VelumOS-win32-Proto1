#include "pm_int.h"

static size_t	field_end(const char *s, size_t n, size_t from)
{
	while (from < n && s[from] != ';')
		from++;
	return (from);
}

static int	parse_tail(const char *s, size_t n, t_pminfo *out)
{
	size_t	act_n;

	if (n < 3 + 1 + 2 * PM_CERT_LEN)
		return (E_INVAL);
	act_n = n - 1 - 2 * PM_CERT_LEN;
	if (s[act_n] != ';' || !pm_act_ok(s, act_n))
		return (E_INVAL);
	memcpy(out->activity, s, act_n);
	return (pm_hex_cert(s + act_n + 1, out->cert));
}

static int	parse_line(const char *s, size_t n, t_pminfo *out)
{
	size_t	a;
	size_t	b;
	size_t	c;

	memset(out, 0, sizeof(*out));
	a = field_end(s, n, 0);
	if (a >= n || !pm_pkg_ok(s, a))
		return (E_INVAL);
	b = field_end(s, n, a + 1);
	if (b >= n || pm_dec_u32(s + a + 1, b - a - 1, &out->version_code) < 0)
		return (E_INVAL);
	c = field_end(s, n, b + 1);
	if (c >= n || !pm_label_ok(s + b + 1, c - b - 1))
		return (E_INVAL);
	memcpy(out->package, s, a);
	memcpy(out->label, s + b + 1, c - b - 1);
	return (parse_tail(s + c + 1, n - c - 1, out));
}

static void	take_line(t_pmreg *r, const char *s, size_t n)
{
	t_pminfo	info;

	if (n > 0 && s[n - 1] == '\r')
		n--;
	if (n == 0)
		return ;
	if (n > PM_LINE_MAX || parse_line(s, n, &info) < 0
		|| r->n >= r->max || pm_index(r->e, r->n, info.package) >= 0)
	{
		r->rejected++;
		return ;
	}
	memset(&r->e[r->n], 0, sizeof(r->e[r->n]));
	r->e[r->n].info = info;
	r->n++;
}

int	pm_registry_scan(t_span text, t_pmreg *r)
{
	size_t	start;
	size_t	end;

	if (!r || (!r->e && r->max) || (!text.p && text.len))
		return (E_INVAL);
	r->n = 0;
	r->rejected = 0;
	start = 0;
	while (start < text.len)
	{
		end = start;
		while (end < text.len && text.p[end] != '\n')
			end++;
		take_line(r, (const char *)text.p + start, end - start);
		start = end + 1;
	}
	return ((int)r->n);
}
