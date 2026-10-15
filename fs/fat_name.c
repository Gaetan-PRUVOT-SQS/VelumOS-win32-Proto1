#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

uint8_t	fat_lfn_sum(const uint8_t *raw)
{
	uint8_t		sum;
	uint32_t	i;

	sum = 0;
	i = 0;
	while (i < 11)
	{
		sum = (uint8_t)(((sum & 1) << 7) + (sum >> 1) + raw[i]);
		i++;
	}
	return (sum);
}

static char	sname_char(uint8_t c, int lower)
{
	if (c < 0x20 || c >= 0x7f || c == '/' || c == '\\')
		return ('_');
	if (lower && c >= 'A' && c <= 'Z')
		return ((char)(c - 'A' + 'a'));
	return ((char)c);
}

static uint32_t	trimmed(const uint8_t *p, uint32_t n)
{
	while (n && p[n - 1] == ' ')
		n--;
	return (n);
}

void	fat_sname_decode(const uint8_t *raw, char *out)
{
	uint32_t	bn;
	uint32_t	en;
	uint32_t	i;
	uint32_t	o;

	bn = trimmed(raw, 8);
	en = trimmed(raw + 8, 3);
	o = 0;
	i = 0;
	while (i < bn)
	{
		if (i == 0 && raw[0] == 0x05)
			out[o++] = '_';
		else
			out[o++] = sname_char(raw[i], raw[12] & 0x08);
		i++;
	}
	if (en)
		out[o++] = '.';
	i = 0;
	while (i < en)
		out[o++] = sname_char(raw[8 + i++], raw[12] & 0x10);
	out[o] = '\0';
}

int	fat_name_valid(const char *name)
{
	size_t	n;
	size_t	i;

	n = strlen(name);
	if (n == 0 || n > VFS_NAME_MAX)
		return (E_INVAL);
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return (E_INVAL);
	if (name[0] == ' ' || name[n - 1] == ' ' || name[n - 1] == '.')
		return (E_INVAL);
	i = 0;
	while (i < n)
	{
		if ((uint8_t)name[i] < 0x20 || name[i] == 0x7f
			|| strchr("\"*/:<>?\\|", name[i]))
			return (E_INVAL);
		i++;
	}
	return (0);
}
