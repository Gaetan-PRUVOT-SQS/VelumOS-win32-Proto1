#include <stdarg.h>
#include <string.h>
#include "fake.h"
#include "velum/klog.h"

void	klog_warn(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fklog_vappend(fmt, ap);
	va_end(ap);
}

void	klog_err(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fklog_vappend(fmt, ap);
	va_end(ap);
}

void	fklog_reset(void)
{
	g_fklog.len = 0;
	g_fklog.buf[0] = '\0';
}

int	fklog_has(const char *s)
{
	return (strstr(g_fklog.buf, s) != NULL);
}
