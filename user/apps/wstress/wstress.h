#ifndef WSTRESS_H
# define WSTRESS_H

# include <stdint.h>

# define WSTRESS_WARMUP 20
# define WSTRESS_CYCLES 320
# define WSTRESS_REPORT 40
# define WSTRESS_WIDTH 400
# define WSTRESS_HEIGHT 300
# define WSTRESS_RETRIES 300
# define WSTRESS_RETRY_NS 10000000ull
# define WSTRESS_SETTLE_NS 500000000ull
# define WSTRESS_SERVER "/system/bin/winsrv"

int			wstress_cycle(void);
uint32_t	wstress_free_kb(void);

#endif
