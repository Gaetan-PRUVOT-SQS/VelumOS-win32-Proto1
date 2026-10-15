#include "velum/klog.h"

#define KLOG_SINKS 4

static t_klogsink	g_sinks[KLOG_SINKS];
static t_loglevel	g_level = LOG_INFO;

void	klog_set_level(t_loglevel level)
{
	g_level = level;
}

int	klog_enabled(t_loglevel level)
{
	return (level <= g_level);
}

void	klog_add_sink(t_klogsink sink)
{
	int	i;

	i = 0;
	while (i < KLOG_SINKS && g_sinks[i])
		i++;
	if (i < KLOG_SINKS)
		g_sinks[i] = sink;
}

void	klog_to_sinks(const char *s, size_t n)
{
	int	i;

	i = 0;
	while (i < KLOG_SINKS)
	{
		if (g_sinks[i])
			g_sinks[i](s, n);
		i++;
	}
}
