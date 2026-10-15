#ifndef WEAK_EXT_H
# define WEAK_EXT_H

# include "velum/ksyscall.h"
# include "velum/proc.h"

extern int			syscall_register(uint32_t num, t_sysfn fn,
						const char *name) __attribute__((weak));
extern t_process	*proc_current(void) __attribute__((weak));

#endif
