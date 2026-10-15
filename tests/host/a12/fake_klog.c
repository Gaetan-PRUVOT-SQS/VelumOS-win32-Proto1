#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "fake.h"
#include "velum/klog.h"

static uint64_t	g_warns;

uint64_t	fake_warns(void)
{
	return (g_warns);
}

static void	klog_any(const char *lvl, const char *fmt, va_list ap)
{
	if (getenv("A12_LOG") == NULL)
		return ;
	fprintf(stderr, "[%s] ", lvl);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
}

void	klog_info(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	klog_any("info", fmt, ap);
	va_end(ap);
}

void	klog_warn(const char *fmt, ...)
{
	va_list	ap;

	g_warns++;
	va_start(ap, fmt);
	klog_any("warn", fmt, ap);
	va_end(ap);
}

void	klog_err(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	klog_any("err", fmt, ap);
	va_end(ap);
}
