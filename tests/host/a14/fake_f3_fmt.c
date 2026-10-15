#include <stdarg.h>
#include "fake_f3.h"
#include "stdio.h"

int	f3_fmt(char *buf, size_t size, const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vsnprintf(buf, size, fmt, ap);
	va_end(ap);
	return (n);
}
