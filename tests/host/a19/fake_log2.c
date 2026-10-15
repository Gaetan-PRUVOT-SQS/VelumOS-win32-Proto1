#include "fake.h"

t_fakecall	*fake_log_get(int i)
{
	if (i < 0 || i >= g_log.n)
		return (NULL);
	return (&g_log.calls[i]);
}

int	fake_log_kind(int kind)
{
	int	i;
	int	n;

	i = 0;
	n = 0;
	while (i < g_log.n)
	{
		if (g_log.calls[i].kind == kind)
			n++;
		i++;
	}
	return (n);
}

t_fakecall	*fake_log_last(int kind)
{
	int	i;

	i = g_log.n - 1;
	while (i >= 0)
	{
		if (g_log.calls[i].kind == kind)
			return (&g_log.calls[i]);
		i--;
	}
	return (NULL);
}
