#include <stdarg.h>
#include <stdio.h>
#include "a04_fake.h"
#include "velum/klog.h"

static int	g_errors;
static int	g_quiet;

int	kprintf(const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);
	return (n);
}

void	klog_info(const char *fmt, ...)
{
	(void)fmt;
}

void	klog_err(const char *fmt, ...)
{
	va_list	ap;

	g_errors++;
	if (g_quiet)
		return ;
	va_start(ap, fmt);
	fputs("klog_err : ", stderr);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);
}

int	fake_log_errors(void)
{
	return (g_errors);
}

void	fake_klog_quiet(int quiet)
{
	g_quiet = quiet;
}
