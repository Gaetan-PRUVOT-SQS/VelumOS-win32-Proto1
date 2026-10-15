#include "stdio.h"
#include "velum/vmisc.h"

int	vprintf(const char *fmt, va_list ap)
{
	return (v_vlogf(V_LOG_INFO, fmt, ap));
}

int	printf(const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = v_vlogf(V_LOG_INFO, fmt, ap);
	va_end(ap);
	return (n);
}

int	puts(const char *s)
{
	return (v_logf(V_LOG_INFO, "%s", s) >= 0);
}
