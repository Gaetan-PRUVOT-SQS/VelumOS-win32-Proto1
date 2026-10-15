#ifndef FAKE_INT_H
# define FAKE_INT_H

# include <pthread.h>
# include <stdint.h>
# include <time.h>
# include "velum/vtls.h"

# define FAKE_MAPS 8192
# define FAKE_EVENTS 128
# define FAKE_THREADS 64
# define FAKE_EV_BASE 0x100
# define FAKE_TH_BASE 0x400
# define FAKE_SLOTS 1024

typedef int64_t	(*t_fhandler)(const uint64_t *a);

typedef struct s_fentry
{
	uint64_t	num;
	t_fhandler	fn;
}	t_fentry;

typedef struct s_fmap
{
	uint64_t	base;
	uint64_t	len;
}	t_fmap;

typedef struct s_fev
{
	int			used;
	int			manual;
	int			signaled;
	int			waiters;
	uint64_t	gen;
}	t_fev;

typedef struct s_fevs
{
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	t_fev			ev[FAKE_EVENTS];
}	t_fevs;

typedef struct s_fslot
{
	t_vtcb		def;
	t_vtcb		*override;
	int64_t		tid;
}	t_fslot;

typedef struct s_fthr
{
	int		used;
	void	(*entry)(void *);
	void	*arg;
}	t_fthr;

extern t_fevs	g_fev;

int64_t			fake_valloc(const uint64_t *a);
int64_t			fake_vfree(const uint64_t *a);
int64_t			fake_vprotect(const uint64_t *a);
int64_t			fake_event_create(const uint64_t *a);
int64_t			fake_event_op(const uint64_t *a);
int64_t			fake_wait(const uint64_t *a);
int64_t			fake_close(const uint64_t *a);
int64_t			fake_event_close(uint64_t h);
int64_t			fake_thread_close(uint64_t h);
int64_t			fake_thread_create(const uint64_t *a);
int64_t			fake_thread_exit(const uint64_t *a);
int64_t			fake_set_fsbase(const uint64_t *a);
int64_t			fake_gettid(const uint64_t *a);
int64_t			fake_log(const uint64_t *a);
int64_t			fake_getrandom(const uint64_t *a);
int64_t			fake_yield(const uint64_t *a);
int64_t			fake_exit(const uint64_t *a);
void			fake_deadline(uint64_t timeout_ns, struct timespec *ts);
t_fev			*fake_ev_get(uint64_t h);
t_fthr			*fake_thr(int i);
pthread_mutex_t	*fake_thr_lock(void);
t_fslot			*fake_slot(void);

#endif
