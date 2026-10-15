#include <stdarg.h>
#include "velum/arch.h"
#include "velum/klog.h"
#include "velum/libk.h"

#define KLOG_LINE 256

void	kputs(const char *s)
{
	size_t	n;

	n = strlen(s);
	serial_write(s, n);
	klog_to_sinks(s, n);
}

void	kputc(char c)
{
	char	s[2];

	s[0] = c;
	s[1] = '\0';
	kputs(s);
}

int	kprintf(const char *fmt, ...)
{
	char	line[KLOG_LINE];
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = kvsnprintf(line, sizeof(line), fmt, ap);
	va_end(ap);
	kputs(line);
	return (n);
}
