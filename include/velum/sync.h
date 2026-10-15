#ifndef SYNC_H
# define SYNC_H

# include <stdbool.h>
# include <stdint.h>

struct	s_thread;

typedef struct s_spinlock
{
	volatile uint32_t	ticket;
	volatile uint32_t	serving;
	const char			*name;
}	t_spinlock;

typedef struct s_waitq
{
	t_spinlock		lock;
	struct s_thread	*head;
	struct s_thread	*tail;
}	t_waitq;

typedef struct s_mutex
{
	t_spinlock		lock;
	struct s_thread	*owner;
	t_waitq			wq;
	const char		*name;
}	t_mutex;

typedef struct s_event
{
	t_spinlock	lock;
	t_waitq		wq;
	bool		signaled;
	bool		manual;
}	t_event;

typedef struct s_sem
{
	t_spinlock	lock;
	t_waitq		wq;
	int64_t		count;
}	t_sem;

void		spin_init(t_spinlock *l, const char *name);
void		spin_lock(t_spinlock *l);
void		spin_unlock(t_spinlock *l);
bool		spin_trylock(t_spinlock *l);
uint64_t	spin_lock_irqsave(t_spinlock *l);
void		spin_unlock_irqrestore(t_spinlock *l, uint64_t flags);
void		waitq_init(t_waitq *wq);
int			waitq_wait(t_waitq *wq, t_spinlock *held, uint64_t timeout_ns);
void		waitq_wake_one(t_waitq *wq);
void		waitq_wake_all(t_waitq *wq);
void		mutex_init(t_mutex *m, const char *name);
void		mutex_lock(t_mutex *m);
bool		mutex_trylock(t_mutex *m);
void		mutex_unlock(t_mutex *m);
void		event_init(t_event *ev, bool manual, bool initial);
void		event_set(t_event *ev);
void		event_reset(t_event *ev);
int			event_wait(t_event *ev, uint64_t timeout_ns);
void		sem_init(t_sem *s, int64_t count);
int			sem_wait(t_sem *s, uint64_t timeout_ns);
void		sem_post(t_sem *s);
int			spin_rank_register(const char *name, uint32_t rank);

#endif
