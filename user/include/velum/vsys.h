#ifndef VSYS_H
# define VSYS_H

# include <stdint.h>

# define V_SYS_ARGS 6

int64_t	v_syscall6(uint64_t num, const uint64_t *args);

#endif
