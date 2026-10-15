#include <stdarg.h>
#include <stdio.h>
#include "velum/klog.h"
#include "a02_fake.h"

void	klog_err(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fake_log_capture(fmt, ap);
	va_end(ap);
}

void	klog_warn(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fake_log_capture(fmt, ap);
	va_end(ap);
}

void	klog_debug(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	va_end(ap);
	(void)fmt;
}

int	kprintf(const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vfprintf(stderr, fmt, ap);
	va_end(ap);
	return (n);
}
