#include "vfs_int.h"
#include "velum/err.h"

static int	utf8_lead(uint8_t c, uint32_t *cp, uint32_t *min)
{
	if (c < 0x80)
	{
		*cp = c;
		*min = 0;
		return (0);
	}
	*min = 0x80;
	*cp = c & 0x1f;
	if (c >= 0xc2 && c <= 0xdf)
		return (1);
	*min = 0x800;
	*cp = c & 0x0f;
	if (c >= 0xe0 && c <= 0xef)
		return (2);
	*min = 0x10000;
	*cp = c & 0x07;
	if (c >= 0xf0 && c <= 0xf4)
		return (3);
	return (-1);
}

static int	utf8_cp_ok(uint32_t cp, uint32_t min)
{
	if (cp < min || cp > 0x10ffff)
		return (0);
	if (cp >= 0xd800 && cp <= 0xdfff)
		return (0);
	if (cp < 0x20 || cp == 0x7f)
		return (0);
	if (cp >= 0x80 && cp <= 0x9f)
		return (0);
	return (1);
}

static int	utf8_one(const uint8_t *s, size_t n, size_t *len)
{
	uint32_t	cp;
	uint32_t	min;
	int			more;
	int			i;

	more = utf8_lead(s[0], &cp, &min);
	if (more < 0 || (size_t)more >= n)
		return (E_INVAL);
	i = 1;
	while (i <= more)
	{
		if ((s[i] & 0xc0) != 0x80)
			return (E_INVAL);
		cp = (cp << 6) | (s[i] & 0x3f);
		i++;
	}
	if (!utf8_cp_ok(cp, min))
		return (E_INVAL);
	*len = (size_t)more + 1;
	return (0);
}

int	vpath_utf8_ok(const uint8_t *s, size_t n)
{
	size_t	i;
	size_t	len;

	i = 0;
	while (i < n)
	{
		if (utf8_one(s + i, n - i, &len) < 0)
			return (E_INVAL);
		i += len;
	}
	return (0);
}
