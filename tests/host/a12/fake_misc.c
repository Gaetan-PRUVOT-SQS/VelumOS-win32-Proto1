#include "fake.h"
#include "velum/libk.h"

static uint64_t		g_fake_now = 1760000000ull * 1000000000ull;
static t_bootinfo	g_fake_boot;

uint64_t	time_wall_ns(void)
{
	return (g_fake_now);
}

void	fake_time_set(uint64_t ns)
{
	g_fake_now = ns;
}

t_bootinfo	*fake_boot(void)
{
	return (&g_fake_boot);
}

const t_bootinfo	*boot_info(void)
{
	return (&g_fake_boot);
}

void	fake_append(char *dst, size_t cap, const char *s)
{
	size_t	n;

	n = 0;
	while (n < cap && dst[n])
		n++;
	while (n + 1 < cap && *s)
		dst[n++] = *s++;
	if (n < cap)
		dst[n] = '\0';
}

const char	*boot_cmdline_get(const char *key)
{
	static char	val[64];
	size_t		n;
	const char	*c;

	n = strlen(key);
	c = g_fake_boot.cmdline;
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
		while (*c == ' ')
			c++;
	}
	return (NULL);
}
