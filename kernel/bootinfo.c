#include "velum/boot.h"
#include "velum/libk.h"

static t_bootinfo	g_info;

t_bootinfo	*boot_info_rw(void)
{
	return (&g_info);
}

const t_bootinfo	*boot_info(void)
{
	return (&g_info);
}

static const char	*word_start(const char *c)
{
	while (*c == ' ')
		c++;
	return (c);
}

int	boot_cmdline_has(const char *word)
{
	const char	*c;
	size_t		n;

	n = strlen(word);
	c = word_start(g_info.cmdline);
	while (*c)
	{
		if (!strncmp(c, word, n) && (c[n] == ' ' || !c[n]))
			return (1);
		while (*c && *c != ' ')
			c++;
		c = word_start(c);
	}
	return (0);
}

const char	*boot_cmdline_get(const char *key)
{
	static char	val[64];
	const char	*c;
	size_t		n;

	n = strlen(key);
	c = word_start(g_info.cmdline);
	while (*c)
	{
		if (!strncmp(c, key, n) && c[n] == '=')
		{
			strlcpy(val, c + n + 1, sizeof(val));
			n = 0;
			while (val[n] && val[n] != ' ')
				n++;
			val[n] = '\0';
			return (val);
		}
		while (*c && *c != ' ')
			c++;
		c = word_start(c);
	}
	return (NULL);
}
