#ifndef KLOG_H
# define KLOG_H

# include <stdarg.h>
# include <stddef.h>

typedef enum e_loglevel
{
	LOG_ERR = 0,
	LOG_WARN = 1,
	LOG_INFO = 2,
	LOG_DEBUG = 3
}	t_loglevel;

typedef void	(*t_klogsink)(const char *s, size_t n);

void	kputs(const char *s);
void	kputc(char c);
int		kprintf(const char *fmt, ...);
int		kvsnprintf(char *buf, size_t size, const char *fmt, va_list ap);
int		ksnprintf(char *buf, size_t size, const char *fmt, ...);
void	klog_info(const char *fmt, ...);
void	klog_warn(const char *fmt, ...);
void	klog_err(const char *fmt, ...);
void	klog_debug(const char *fmt, ...);
void	klog_set_level(t_loglevel level);
int		klog_enabled(t_loglevel level);
void	klog_to_sinks(const char *s, size_t n);
void	klog_add_sink(t_klogsink sink);

#endif
