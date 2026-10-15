#include <stdarg.h>
#include <string.h>
#include "fake.h"
#include "velum/klog.h"

t_fklog	g_fklog;

void	fklog_vappend(const char *fmt, va_list ap)
{
	char	line[256];
	size_t	n;

	kvsnprintf(line, sizeof(line), fmt, ap);
	n = strlen(line);
	if (g_fklog.len + n + 2 >= FKLOG_SIZE)
		return ;
	memcpy(&g_fklog.buf[g_fklog.len], line, n);
	g_fklog.len += n;
	g_fklog.buf[g_fklog.len] = '\n';
	g_fklog.len++;
	g_fklog.buf[g_fklog.len] = '\0';
}

int	kprintf(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fklog_vappend(fmt, ap);
	va_end(ap);
	return (0);
}

void	klog_info(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fklog_vappend(fmt, ap);
	va_end(ap);
}
