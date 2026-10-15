#include "stdio.h"
#include "velum/klog.h"

int	vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
	return (kvsnprintf(buf, size, fmt, ap));
}

int	snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vsnprintf(buf, size, fmt, ap);
	va_end(ap);
	return (n);
}
