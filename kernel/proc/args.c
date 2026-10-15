#include "proc_pure.h"
#include "velum/abi/abi_types.h"
#include "velum/err.h"
#include "velum/proc.h"

static uint32_t	utf8_len(uint8_t c)
{
	if (c < 0x80)
		return (1);
	if ((c & 0xe0) == 0xc0)
		return (2);
	if ((c & 0xf0) == 0xe0)
		return (3);
	if ((c & 0xf8) == 0xf0)
		return (4);
	return (0);
}

static uint32_t	utf8_seq(const uint8_t *s, uint64_t rem)
{
	uint32_t	n;
	uint32_t	i;
	uint32_t	cp;

	n = utf8_len(s[0]);
	if (n == 1)
		return (s[0] >= 0x20 && s[0] != 0x7f);
	if (n == 0 || n > rem)
		return (0);
	cp = s[0] & (0x7f >> n);
	i = 1;
	while (i < n)
	{
		if ((s[i] & 0xc0) != 0x80)
			return (0);
		cp = (cp << 6) | (s[i] & 0x3f);
		i++;
	}
	if ((n == 2 && cp < 0xa0) || (n == 3 && cp < 0x800)
		|| (n == 4 && cp < 0x10000))
		return (0);
	if ((cp >= 0xd800 && cp <= 0xdfff) || cp > 0x10ffff)
		return (0);
	return (n);
}

int	path_check(const char *path, uint64_t len)
{
	uint64_t	i;
	uint32_t	n;

	if (!path || len == 0 || len >= VFS_PATH_MAX || path[0] != '/')
		return (E_INVAL);
	i = 0;
	while (i < len)
	{
		n = utf8_seq((const uint8_t *)path + i, len - i);
		if (n == 0)
			return (E_INVAL);
		i += n;
	}
	return (0);
}

int	args_count(const char *args, uint64_t len)
{
	uint64_t	i;
	int			n;

	if (len == 0)
		return (0);
	if (!args || len > PROC_ARGS_MAX || args[len - 1] != '\0')
		return (E_INVAL);
	n = 0;
	i = 0;
	while (i < len)
	{
		if (args[i] == '\0')
			n++;
		i++;
	}
	if (n > PROC_NARGS_MAX)
		return (E_INVAL);
	return (n);
}

void	proc_name_from_path(char *dst, const char *path, uint64_t len)
{
	uint64_t	start;
	uint64_t	i;
	uint64_t	k;

	start = 0;
	i = 0;
	while (i < len && path[i])
	{
		if (path[i] == '/')
			start = i + 1;
		i++;
	}
	k = 0;
	while (start + k < i && k < PROC_NAME_LEN - 1)
	{
		dst[k] = path[start + k];
		k++;
	}
	dst[k] = '\0';
	log_sanitize(dst, k);
	if (k == 0)
	{
		dst[0] = '?';
		dst[1] = '\0';
	}
}
