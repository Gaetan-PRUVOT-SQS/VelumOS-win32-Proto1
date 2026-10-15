#ifndef KSYSCALL_H
# define KSYSCALL_H

# include <stdint.h>
# include "abi/abi_syscall.h"
# include "cpu.h"
# include "uptr.h"

typedef struct s_sysargs
{
	uint64_t	a[6];
	t_regs		*regs;
}	t_sysargs;

typedef int64_t	(*t_sysfn)(const t_sysargs *args);

int			syscall_boot_init(void);
int			syscall_register(uint32_t num, t_sysfn fn, const char *name);
int64_t		syscall_dispatch(t_sysargs *args, uint32_t num);
uint64_t	syscall_calls(uint32_t num);
uint64_t	syscall_unknown_count(void);
int			syscall_require(uint32_t pf);
int			syscall_selftest(void);
int			syscall_arch_init(void);
int			syscall_arch_selftest(void);

#endif
