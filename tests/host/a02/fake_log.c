#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "velum/klog.h"
#include "a02_fake.h"

static char	g_last[256];
static int	g_count;

void	fake_log_capture(const char *fmt, va_list ap)
{
	vsnprintf(g_last, sizeof(g_last), fmt, ap);
	g_count++;
}

void	klog_info(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fake_log_capture(fmt, ap);
	va_end(ap);
}

const char	*fake_last_log(void)
{
	return (g_last);
}

int	fake_log_count(void)
{
	return (g_count);
}

void	fake_log_reset(void)
{
	g_count = 0;
	memset(g_last, 0, sizeof(g_last));
}
