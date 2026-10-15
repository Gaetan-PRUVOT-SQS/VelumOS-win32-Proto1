#ifndef SYS_INT_H
# define SYS_INT_H

# include <stddef.h>
# include <stdint.h>
# include "velum/err.h"
# include "velum/vsys.h"

static inline int64_t	sys3(uint64_t num, uint64_t a0, uint64_t a1,
		uint64_t a2)
{
	uint64_t	args[V_SYS_ARGS];

	args[0] = a0;
	args[1] = a1;
	args[2] = a2;
	args[3] = 0;
	args[4] = 0;
	args[5] = 0;
	return (v_syscall6(num, args));
}

static inline int64_t	sys2(uint64_t num, uint64_t a0, uint64_t a1)
{
	return (sys3(num, a0, a1, 0));
}

static inline int64_t	sys1(uint64_t num, uint64_t a0)
{
	return (sys3(num, a0, 0, 0));
}

static inline int64_t	sys0(uint64_t num)
{
	return (sys3(num, 0, 0, 0));
}

static inline uint64_t	sys_ptr(const void *p)
{
	return ((uint64_t)(uintptr_t)p);
}

int64_t	sys_cstrlen(const char *s, size_t max);

#endif
