#ifndef VMISC_H
# define VMISC_H

# include <stdarg.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/abi/abi_syscall.h"
# include "velum/abi/abi_types.h"

# define V_LOG_ERR 0
# define V_LOG_WARN 1
# define V_LOG_INFO 2
# define V_LOG_DEBUG 3
# define V_LOG_MAX 200
# define V_RANDOM_MAX 256

int		v_log(uint32_t level, const char *msg);
int64_t	v_getrandom(void *buf, size_t len, uint32_t flags);
int		v_sysinfo(t_sysinfo *out);
int		v_vlogf(uint32_t level, const char *fmt, va_list ap);
int		v_logf(uint32_t level, const char *fmt, ...)
		__attribute__((format(printf, 2, 3)));

#endif
