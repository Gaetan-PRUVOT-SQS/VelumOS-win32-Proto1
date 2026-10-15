#ifndef VTIME_H
# define VTIME_H

# include <stdint.h>
# include "velum/abi/abi_syscall.h"

# define V_NS_PER_US 1000ull
# define V_NS_PER_MS 1000000ull
# define V_NS_PER_SEC 1000000000ull

int		v_yield(void);
int		v_sleep(uint64_t ns);
int64_t	v_time_mono(void);
int64_t	v_time_wall(void);
int		v_time_set_wall(uint64_t ns);
int		v_power(uint32_t op);

#endif
