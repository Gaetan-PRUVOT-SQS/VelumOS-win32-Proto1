#ifndef EXIT_INT_H
# define EXIT_INT_H

# include <stdint.h>
# include "stdlib.h"
# include "velum/vsync.h"

# define EXIT_ABORT_CODE 134

typedef void	(*t_atexitfn)(void);

typedef struct s_atexit
{
	t_vspin		lock;
	uint32_t	count;
	t_atexitfn	fns[ATEXIT_MAX];
}	t_atexit;

void	exit_run_handlers(void);

#endif
