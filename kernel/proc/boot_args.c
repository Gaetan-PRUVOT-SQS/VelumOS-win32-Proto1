#include "proc_int.h"
#include "velum/boot.h"

static uint32_t	word_len(const char *s, uint32_t rem)
{
	uint32_t	n;

	n = 0;
	while (n < rem && s[n] && s[n] != ' ')
		n++;
	return (n);
}

static int	word_ok(const char *w, uint32_t n)
{
	const char	*prefix;
	uint32_t	i;

	prefix = "init.";
	if (n <= 5 || n > BOOT_ARG_MAX)
		return (0);
	i = 0;
	while (i < n)
	{
		if (i < 5 && w[i] != prefix[i])
			return (0);
		if (w[i] < 0x21 || w[i] > 0x7e)
			return (0);
		i++;
	}
	return (1);
}

static uint32_t	word_put(char *out, const char *w, uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		out[i] = w[i];
		i++;
	}
	out[n] = '\0';
	return (n + 1);
}

uint32_t	boot_args_init(const char *cmdline, char *out, uint32_t cap)
{
	uint32_t	i;
	uint32_t	n;
	uint32_t	len;
	uint32_t	words;

	if (!cmdline || !out)
		return (0);
	i = 0;
	len = 0;
	words = 0;
	while (i < BOOT_CMDLINE_MAX && cmdline[i] && words < BOOT_NARGS_MAX)
	{
		n = word_len(cmdline + i, BOOT_CMDLINE_MAX - i);
		if (i + n < BOOT_CMDLINE_MAX && word_ok(cmdline + i, n)
			&& n < cap - len)
		{
			len += word_put(out + len, cmdline + i, n);
			words++;
		}
		i += n;
		while (i < BOOT_CMDLINE_MAX && cmdline[i] == ' ')
			i++;
	}
	return (len);
}
