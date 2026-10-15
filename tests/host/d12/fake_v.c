#include <string.h>
#include "velum/err.h"
#include "fake.h"

t_fv	g_fv;

void	fv_reset(void)
{
	memset(&g_fv, 0, sizeof(g_fv));
}

int	fv_tick(void)
{
	g_fv.calls++;
	if (g_fv.fail_at == 0)
		return (0);
	if (g_fv.calls == g_fv.fail_at)
		return (E_IO);
	if (g_fv.cut && g_fv.calls > g_fv.fail_at)
		return (E_IO);
	return (0);
}

int	fv_find(const char *name)
{
	int	i;

	i = 0;
	while (i < FV_FILES)
	{
		if (g_fv.f[i].used && strcmp(g_fv.f[i].name, name) == 0)
			return (i);
		i++;
	}
	return (-1);
}

int	fv_put(const char *name, const char *text)
{
	int	i;

	i = fv_find(name);
	if (i < 0)
	{
		i = 0;
		while (i < FV_FILES && g_fv.f[i].used)
			i++;
	}
	if (i >= FV_FILES || strlen(name) >= FV_NAME || strlen(text) > FV_DATA)
		return (E_NOSPC);
	memset(&g_fv.f[i], 0, sizeof(g_fv.f[i]));
	memcpy(g_fv.f[i].name, name, strlen(name) + 1);
	memcpy(g_fv.f[i].data, text, strlen(text));
	g_fv.f[i].len = strlen(text);
	g_fv.f[i].used = 1;
	return (i);
}
