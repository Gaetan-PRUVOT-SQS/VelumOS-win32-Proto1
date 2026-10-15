#ifndef VTEST_H
# define VTEST_H

# include <stdint.h>
# include "velum/velum.h"

# define VTEST_THREADS 4
# define VTEST_LOOPS 5000

typedef struct s_vtest
{
	int	checks;
	int	fails;
}	t_vtest;

typedef struct s_vtest_shared
{
	t_vmutex	lock;
	t_vevent	gate;
	uint64_t	counter;
	int			errno_seen;
}	t_vtest_shared;

extern t_vtest			g_vtest;
extern t_vtest_shared	g_shared;

void	vtest_check(int cond, const char *name);
int		vtest_report(void);
void	vtest_mem(void);
void	vtest_str(void);
void	vtest_sys(void);
void	vtest_files(void);
void	vtest_thr(void);
void	*vtest_worker_add(void *arg);
void	*vtest_worker_gate(void *arg);
void	*vtest_worker_errno(void *arg);

#endif
