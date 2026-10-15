#ifndef INIT_H
# define INIT_H

# include <stdint.h>
# include "velum/abi/abi_syscall.h"
# include "velum/abi/abi_types.h"

# define INIT_SERVICES 2
# define RESTART_MAX 5
# define RESTART_WINDOW_NS 30000000000ull
# define INIT_IDLE_NS 1000000000ull
# define INIT_PROCS_MAX 256
# define KILLTEST_ARG "init.mort-winsrv"
# define KILLTEST_GRACE_NS 3000000000ull
# define KILLTEST_POLL_NS 500000000ull
# define KILLTEST_CODE 99
# define KT_OFF 0
# define KT_ARMED 1
# define KT_GRACE 2
# define KT_DONE 3
# define KT_EXPIRED 4
# define KILLTEST_TICKS_MAX 120

typedef struct s_restarts
{
	uint64_t	stamp[RESTART_MAX];
	uint32_t	n;
	uint32_t	next;
}	t_restarts;

typedef struct s_service
{
	const char	*path;
	uint32_t	flags;
	t_handle	h;
	int			enabled;
	t_restarts	rs;
}	t_service;

typedef struct s_killtest
{
	uint32_t	state;
	uint32_t	ticks;
	uint64_t	seen_ns;
}	t_killtest;

int			restart_allowed(t_restarts *r, uint64_t now_ns);
int			service_start(t_service *s);
void		service_died(t_service *s, uint64_t now_ns);
int			killtest_requested(int argc, char **argv);
int			killtest_step(t_killtest *kt, int session_ready, uint64_t now_ns);
void		killtest_setup(t_killtest *kt, int argc, char **argv);
uint64_t	killtest_timeout(const t_killtest *kt);
int			procs_have(const t_procinfo *ps, uint32_t n, const char *name);
void		killtest_tick(t_killtest *kt, t_service *winsrv, uint64_t now_ns);

#endif
