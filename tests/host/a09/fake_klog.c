#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "fake.h"
#include "velum/klog.h"

static void	fake_vlog(const char *level, const char *fmt, va_list ap)
{
	char	line[512];

	vsnprintf(line, sizeof(line), fmt, ap);
	fake_log_append("%s%s\n", level, line);
}

void	klog_info(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fake_vlog("[info] ", fmt, ap);
	va_end(ap);
}

void	klog_warn(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fake_vlog("[warn] ", fmt, ap);
	va_end(ap);
}

void	klog_err(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fake_vlog("[err] ", fmt, ap);
	va_end(ap);
}

void	klog_debug(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fake_vlog("[debug] ", fmt, ap);
	va_end(ap);
}
