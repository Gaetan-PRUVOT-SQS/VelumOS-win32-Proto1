#include "pm_int.h"

static char	fold(char c)
{
	if (c >= 'A' && c <= 'Z')
		return ((char)(c + 32));
	return (c);
}

int	pm_same_pkg(const char *a, const char *b)
{
	size_t	i;

	i = 0;
	while (i < PM_PKG_MAX && a[i] && fold(a[i]) == fold(b[i]))
		i++;
	return (i < PM_PKG_MAX && a[i] == '\0' && b[i] == '\0');
}

int	pm_dec_u32(const char *s, size_t n, uint32_t *out)
{
	uint64_t	v;
	size_t		i;

	if (n == 0 || n > 10 || (n > 1 && s[0] == '0'))
		return (E_INVAL);
	v = 0;
	i = 0;
	while (i < n)
	{
		if (s[i] < '0' || s[i] > '9')
			return (E_INVAL);
		v = v * 10 + (uint64_t)(s[i] - '0');
		i++;
	}
	if (v > UINT32_MAX)
		return (E_INVAL);
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

int	pm_hex_cert(const char *s, uint8_t *out)
{
	size_t	i;
	int		hi;
	int		lo;

	i = 0;
	while (i < PM_CERT_LEN)
	{
		hi = hex_val(s[2 * i]);
		lo = hex_val(s[2 * i + 1]);
		if (hi < 0 || lo < 0)
			return (E_INVAL);
		out[i] = (uint8_t)(hi * 16 + lo);
		i++;
	}
	return (0);
}
