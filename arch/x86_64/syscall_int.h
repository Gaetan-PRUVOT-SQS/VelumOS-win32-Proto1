#ifndef SYSCALL_INT_H
# define SYSCALL_INT_H

# include <stdint.h>
# include "velum/cpu.h"

# define RFLAGS_USER_MASK 0x240dd5ull
# define RFLAGS_IF 0x200ull
# define RFLAGS_FIXED 0x2ull
# define SYSCALL_SFMASK 0x44700ull
# define SYSCALL_STAR 0x0010000800000000ull
# define EFER_SCE 0x1ull
# define USER_CANON_END 0x0000800000000000ull
# define SYSRET_PATH 1
# define IRET_PATH 0
# define KILL_PATH -1

void		syscall_entry(void);
int			syscall_entry_c(t_regs *regs);
uint64_t	syscall_user_rflags(uint64_t rflags);
int			sysret_ok(const t_regs *r);
int			syscall_prepare_return(t_regs *r);

#endif
