#ifndef FAKE_SCHED_H
# define FAKE_SCHED_H

# include "fake_host.h"
# include "sched_int.h"
# include "velum/timer.h"

# define FT_SLOTS 64
# define FT_FREE 0
# define FT_ARMED 1
# define FT_FIRING 2

typedef struct s_fakethr
{
	t_kthread			kt;
	t_lockstack			ls;
	uint32_t			preempt;
	uint64_t			iflag;
	uint32_t			relax;
	struct s_fakethr	*next;
}	t_fakethr;

typedef struct s_ftimer
{
	uint64_t	deadline;
	t_timerfn	fn;
	void		*ctx;
	int64_t		id;
	int			state;
}	t_ftimer;

typedef struct s_ftab
{
	t_ftimer	slot[FT_SLOTS];
	int64_t		next_id;
	int			fail;
	int			started;
	void		*service;
	t_fakethr	*threads;
	uint32_t	next_tid;
	uint64_t	parks;
}	t_ftab;

typedef struct s_fuwaiter
{
	t_waitq		*wq;
	t_spinlock	*held;
	uint64_t	timeout;
	int32_t		prio;
	t_kthread	*kt;
	int			rc;
	int			tag;
	int			*order;
	int			*norder;
	int			held_ok;
	t_mutex		*m;
	t_event		*ev;
	t_sem		*sem;
	int32_t		seen_prio;
	int			owner_ok;
}	t_fuwaiter;

typedef struct s_futest
{
	t_spinlock			lock;
	t_mutex				m;
	t_event				ping;
	t_event				pong;
	t_sem				sem;
	volatile uint64_t	counter;
	volatile int		flag;
	int					errors;
}	t_futest;

extern t_ftab	g_ft;

t_fakethr	*fake_self(void);
void		ft_start(void);
void		fu_waiter(void *arg);
uint32_t	fu_wq_len(t_waitq *wq);
bool		fu_wait_len(t_waitq *wq, uint32_t n);
void		fu_kt_init(t_kthread *kt, int32_t prio);
void		fu_ev_waiter(void *arg);
void		fu_sem_waiter(void *arg);
void		fu_pi_waiter(void *arg);
void		fu_wait_flag(volatile int *flag, int value);

#endif
