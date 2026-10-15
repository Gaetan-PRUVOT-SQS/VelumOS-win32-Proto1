#include <stdarg.h>
#include <stdio.h>
#include "fake.h"
#include "velum/klog.h"

static void	log_store(const char *fmt, va_list ap)
{
	vsnprintf(g_fk.last, sizeof(g_fk.last), fmt, ap);
}

void	klog_info(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	log_store(fmt, ap);
	va_end(ap);
}

void	klog_warn(const char *fmt, ...)
{
	va_list	ap;

	g_fk.warns++;
	va_start(ap, fmt);
	log_store(fmt, ap);
	va_end(ap);
}

void	klog_err(const char *fmt, ...)
{
	va_list	ap;

	g_fk.errs++;
	va_start(ap, fmt);
	log_store(fmt, ap);
	va_end(ap);
}

void	fk_reset_log(void)
{
	g_fk.warns = 0;
	g_fk.errs = 0;
	g_fk.timeouts = 0;
	g_fk.last[0] = '\0';
}
