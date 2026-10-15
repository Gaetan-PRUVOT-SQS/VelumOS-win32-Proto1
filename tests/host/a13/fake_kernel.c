#include <stdarg.h>
#include <stdio.h>
#include "fakes.h"
#include "velum/klog.h"
#include "velum/panic.h"

t_fk	g_fk;

void	klog_info(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(g_fk.last, sizeof(g_fk.last), fmt, ap);
	va_end(ap);
	g_fk.info++;
}

void	klog_warn(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(g_fk.last, sizeof(g_fk.last), fmt, ap);
	va_end(ap);
	g_fk.warn++;
}

void	klog_add_sink(t_klogsink sink)
{
	g_fk.sink = sink;
	g_fk.sinks++;
}

void	panic_set_screen(void (*fn)(const char *t, const char *m))
{
	g_fk.screen = fn;
	g_fk.screens++;
}
