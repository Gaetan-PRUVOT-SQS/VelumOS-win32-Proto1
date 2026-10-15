#ifndef SCHED_H
# define SCHED_H

# include <stdbool.h>
# include <stdint.h>
# include "sync.h"
# include "vmm.h"

# define PRIO_IDLE 0
# define PRIO_LOW 4
# define PRIO_NORMAL 8
# define PRIO_HIGH 13
# define PRIO_REALTIME 24
# define PRIO_MAX 31
# define TIMEOUT_NONE 0xffffffffffffffffull
# define THREAD_USER 0x1
# define THREAD_DETACHED 0x2

typedef enum e_tstate
{
	TS_NEW = 0,
	TS_READY,
	TS_RUNNING,
	TS_BLOCKED,
	TS_ZOMBIE
}	t_tstate;

typedef struct s_thread
{
	uint32_t			tid;
	char				name[24];
	uint32_t			state;
	int32_t				prio;
	int32_t				base_prio;
	uint32_t			flags;
	uint64_t			rsp;
	uint64_t			kstack_base;
	uint64_t			kstack_top;
	t_aspace			*aspace;
	struct s_process	*proc;
	void				*fpu;
	uint64_t			fs_base;
	uint64_t			wake_ns;
	int32_t				wait_result;
	int32_t				exit_code;
	uint64_t			cpu_time_ns;
	uint32_t			refs;
	uint32_t			quantum_left;
	struct s_thread		*next;
	struct s_thread		*prev;
	struct s_thread		*proc_next;
	struct s_object		*obj;
	t_waitq				joiners;
}	t_thread;

typedef void	(*t_threadfn)(void *arg);

typedef struct s_threadreq
{
	const char			*name;
	t_threadfn			fn;
	void				*arg;
	int32_t				prio;
	uint32_t			flags;
	struct s_process	*proc;
	uint64_t			entry;
	uint64_t			sp;
}	t_threadreq;

int				sched_boot_init(void);
t_thread		*sched_current(void);
t_thread		*thread_create(const t_threadreq *rq);
_Noreturn void	thread_exit(int code);
int				thread_join(t_thread *t, uint64_t timeout_ns);
void			thread_ref(t_thread *t);
void			thread_unref(t_thread *t);
void			thread_set_prio(t_thread *t, int prio);
void			sched_yield(void);
void			sched_sleep_ns(uint64_t ns);
void			sched_block(void);
void			sched_wake(t_thread *t);
void			sched_preempt_disable(void);
void			sched_preempt_enable(void);
void			sched_tick(uint64_t now_ns);
void			sched_idle(void);
void			sched_irq_exit(void);
void			sched_set_quantum_ns(uint64_t ns);
void			thread_cancel(t_thread *t);
bool			thread_cancel_pending(void);

#endif
