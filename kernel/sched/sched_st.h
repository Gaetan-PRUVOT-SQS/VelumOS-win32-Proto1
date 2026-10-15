#ifndef SCHED_ST_H
# define SCHED_ST_H

# include <stdint.h>
# include "sched_int.h"

# define ST_MS 1000000ull
# define ST_FAIR_THREADS 4
# define ST_FAIR_RUN_NS 600000000ull
# define ST_FAIR_TOLERANCE_PCT 20
# define ST_PREEMPT_SLEEP_NS 30000000ull
# define ST_PREEMPT_MAX_NS 5000000ull
# define ST_SLEEP_NS 20000000ull
# define ST_SLEEP_TOL_NS 10000000ull
# define ST_PP_ROUNDS 1000
# define ST_MUTEX_THREADS 3
# define ST_MUTEX_LOOPS 2000
# define ST_CHURN 10000
# define ST_CHURN_WARMUP 64
# define ST_JOIN_NS 2000000000ull

typedef struct s_stfair
{
	volatile int32_t	stop;
	t_thread			*t[ST_FAIR_THREADS];
}	t_stfair;

typedef struct s_stpre
{
	volatile int32_t	flag;
	uint64_t			target_ns;
	uint64_t			woke_ns;
	uint64_t			seen_ns;
}	t_stpre;

typedef struct s_stpp
{
	t_event		ping;
	t_event		pong;
	int32_t		count;
	int32_t		errors;
}	t_stpp;

typedef struct s_stmx
{
	t_mutex		m;
	int32_t		counter;
}	t_stmx;

typedef struct s_stsnap
{
	uint64_t	heap_objs;
	uint64_t	heap_bytes;
	uint64_t	pmm_free;
}	t_stsnap;

void	st_snapshot(t_stsnap *out);
int		st_reaper_drain(void);
int		st_join_unref(t_thread *t);

#endif
