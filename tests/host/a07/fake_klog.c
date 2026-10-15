#include "a07_fake.h"

t_fakelog	g_fakelog = {0};

void	klog_warn(const char *fmt, ...)
{
	(void)fmt;
	g_fakelog.lines++;
}

void	klog_info(const char *fmt, ...)
{
	(void)fmt;
	g_fakelog.lines++;
}

void	klog_err(const char *fmt, ...)
{
	(void)fmt;
	g_fakelog.lines++;
}

void	klog_debug(const char *fmt, ...)
{
	(void)fmt;
	g_fakelog.lines++;
}
