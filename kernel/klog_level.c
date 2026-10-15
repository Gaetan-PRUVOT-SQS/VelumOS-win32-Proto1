#include <stdarg.h>
#include "velum/irqflags.h"
#include "velum/klog.h"

#define KLOG_LINE 256

extern uint64_t	time_now_ns(void) __attribute__((weak));

static void	log_emit(t_loglevel lv, const char *fmt, va_list ap)
{
	static const char	tag[4] = {'E', 'W', 'I', 'D'};
	char				body[KLOG_LINE];
	uint64_t			ns;
	uint64_t			fl;

	if (!klog_enabled(lv))
		return ;
	ns = 0;
	if (time_now_ns)
		ns = time_now_ns();
	kvsnprintf(body, sizeof(body), fmt, ap);
	fl = irq_save();
	kprintf("[%5llu.%03llu] %c %s\n", ns / 1000000000ull,
		ns / 1000000ull % 1000, tag[lv], body);
	irq_restore(fl);
}

void	klog_info(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	log_emit(LOG_INFO, fmt, ap);
	va_end(ap);
}

void	klog_warn(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	log_emit(LOG_WARN, fmt, ap);
	va_end(ap);
}

void	klog_err(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	log_emit(LOG_ERR, fmt, ap);
	va_end(ap);
}

void	klog_debug(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	log_emit(LOG_DEBUG, fmt, ap);
	va_end(ap);
}
