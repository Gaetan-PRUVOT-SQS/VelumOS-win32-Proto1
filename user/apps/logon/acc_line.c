#include "velum/err.h"
#include "../common/utf8.h"
#include "accounts.h"

int	acc_split(const char *line, size_t len, t_field *f)
{
	size_t	i;
	size_t	start;
	int		n;

	i = 0;
	start = 0;
	n = 0;
	while (i <= len)
	{
		if (i == len || line[i] == ':')
		{
			if (n >= ACC_FIELDS)
				return (0);
			f[n].p = line + start;
			f[n].n = i - start;
			n++;
			start = i + 1;
		}
		i++;
	}
	return (n == ACC_FIELDS);
}

int	acc_dec(const t_field *f, uint32_t *out)
{
	uint64_t	v;
	size_t		i;

	if (f->n == 0 || f->n > 10)
		return (E_INVAL);
	v = 0;
	i = 0;
	while (i < f->n)
	{
		if (f->p[i] < '0' || f->p[i] > '9')
			return (E_INVAL);
		v = v * 10 + (uint64_t)(f->p[i] - '0');
		i++;
	}
	if (v > UINT32_MAX)
		return (E_RANGE);
	*out = (uint32_t)v;
	return (0);
}

static int	hex_val(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (-1);
}

int	acc_hex(const t_field *f, uint8_t *out, size_t max)
{
	size_t	i;
	int		hi;
	int		lo;

	if (f->n == 0 || f->n % 2 != 0 || f->n / 2 > max)
		return (E_INVAL);
	i = 0;
	while (i < f->n / 2)
	{
		hi = hex_val(f->p[2 * i]);
		lo = hex_val(f->p[2 * i + 1]);
		if (hi < 0 || lo < 0)
			return (E_INVAL);
		out[i] = (uint8_t)(hi * 16 + lo);
		i++;
	}
	return ((int)(f->n / 2));
}

int	acc_name_ok(const char *s, size_t n)
{
	size_t	i;

	if (n == 0 || n >= ACC_NAME_MAX || s[0] == ' ' || s[n - 1] == ' ')
		return (0);
	i = 0;
	while (i < n)
	{
		if ((uint8_t)s[i] < 0x20 || s[i] == 0x7f)
			return (0);
		i++;
	}
	return (utf8_valid(s, n));
}
