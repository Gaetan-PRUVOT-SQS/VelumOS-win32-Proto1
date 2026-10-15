#ifndef SCHED_INT_H
# define SCHED_INT_H

# include <stdbool.h>
# include <stdint.h>
# include "velum/cpu.h"
# include "velum/sched.h"
# include "velum/sync.h"

# ifdef VELUM_DEBUG
#  define SCHED_DEBUG 1
# else
#  define SCHED_DEBUG 0
# endif

# define PRIO_LEVELS 32
# define PRIO_AGED 23
# define BOOST_WAKE 2
# define KSTACK_PAGES 4
# define QUANTUM_NS 15000000ull
# define QUANTUM_MIN_NS 1000000ull
# define QUANTUM_MAX_NS 1000000000ull
# define AGING_NS 1000000000ull
# define SPIN_STUCK_NS 1000000000ull
# define SPIN_CHECK_MASK 1023
# define LOCKDEP_DEPTH 16
# define RANK_SLOTS 32
# define RANK_MUTEX 10
# define RANK_WAITQ 30
# define RANK_RUNQ 40
# define KT_STATIC 0x1
# define KT_IDLE 0x2
# define SR_YIELD 0
# define SR_PREEMPT 1
# define SR_BLOCK 2
# define SR_EXIT 3
# define RFLAGS_IF 0x200

typedef struct s_kthread
{
	t_thread			t;
	t_waitq				*wq;
	int32_t				reason;
	int32_t				cancelable;
	int32_t				canceled;
	int32_t				token;
	int32_t				boost;
	int32_t				inherit;
	int32_t				exited;
	int32_t				tmo_done;
	int64_t				tmo_id;
	uint32_t			kflags;
	uint32_t			irq_depth;
	uint64_t			user_rsp;
	uint64_t			ready_ns;
	t_threadfn			fn;
	void				*arg;
	uint64_t			entry;
	uint64_t			user_sp;
	struct s_kthread	*reap_next;
}	t_kthread;

typedef struct s_held
{
	t_spinlock	*lock;
	void		*caller;
	uint32_t	rank;
}	t_held;

typedef struct s_lockstack
{
	t_held		held[LOCKDEP_DEPTH];
	uint32_t	depth;
}	t_lockstack;

typedef struct s_runq
{
	uint32_t	bitmap;
	uint32_t	count;
	t_kthread	*head[PRIO_LEVELS];
	t_kthread	*tail[PRIO_LEVELS];
}	t_runq;

typedef struct s_cpusched
{
	t_spinlock	lock;
	t_runq		rq;
	t_cpu		*cpu;
	t_kthread	*idle;
	t_kthread	*reaper;
	t_kthread	*dead;
	t_aspace	*active_as;
	uint64_t	slice_start;
	uint64_t	quantum_ns;
	uint64_t	switches;
	int64_t		qtimer_id;
	int32_t		qtimer_pending;
	int32_t		need_resched;
	t_lockstack	locks;
}	t_cpusched;

typedef struct s_rankent
{
	const char	*name;
	uint32_t	rank;
}	t_rankent;

typedef struct s_ranktab
{
	t_rankent	ent[RANK_SLOTS];
	uint32_t	count;
	uint32_t	busy;
}	t_ranktab;

typedef struct s_schedglob
{
	t_cpusched	bsp;
	t_kthread	boot;
	int32_t		ready;
	uint32_t	next_tid;
}	t_schedglob;

extern t_schedglob	g_sched;

void			spin_acquire(t_spinlock *l, void *caller);
void			spin_release(t_spinlock *l, void *caller);
void			lockdep_acquire(t_spinlock *l, void *caller, bool check);
uint32_t		lockdep_rank_of(const char *name);
void			lockdep_release(t_spinlock *l);
uint32_t		lockdep_depth(void);
t_lockstack		*sched_lockstack(void);
uint32_t		sched_preempt_count(void);
t_kthread		*sched_kself(void);
t_cpusched		*sched_cpu(void);
void			sched_park(void);
void			sched_park_prepare(t_kthread *kt);
void			sched_unpark(t_kthread *kt, bool boost);
void			sched_unpark_locked(t_cpusched *cs, t_kthread *kt, bool boost);
void			sched_prio_inherit(t_kthread *owner, int32_t prio);
void			sched_prio_uninherit(t_kthread *kt);
void			sched_resched(int why);
void			sched_pass_locked(t_cpusched *cs, int why);
void			sched_finish(t_cpusched *cs);
void			sched_reprio_locked(t_cpusched *cs, t_kthread *kt);
void			sched_thread_entry(t_kthread *kt);
void			sched_exit_current(void);
void			sched_syscalls_register(void);
void			sched_reaper_main(void *arg);
void			sched_reap_push_locked(t_cpusched *cs, t_kthread *kt);
void			sched_idle_main(void *arg);
void			sched_quantum_cb(void *ctx);
t_kthread		*thread_alloc(const t_threadreq *rq);
void			thread_free(t_kthread *kt);
void			waitq_list_append(t_waitq *wq, t_kthread *kt);
void			waitq_list_remove(t_waitq *wq, t_kthread *kt);
t_kthread		*waitq_list_pop(t_waitq *wq);
int32_t			waitq_list_max_prio(t_waitq *wq);
int				wq_block(t_waitq *wq, t_spinlock *held, uint64_t deadline,
					bool cancelable);
bool			wq_enter(t_waitq *wq, t_kthread *kt, bool cancelable);
void			wq_arm(t_kthread *kt, uint64_t deadline);
void			wq_disarm(t_kthread *kt);
t_kthread		*wq_wake_one(t_waitq *wq, int32_t result);
void			wq_wake_all_locked(t_waitq *wq, int32_t result);
void			runq_init(t_runq *rq);
void			runq_push(t_runq *rq, t_kthread *kt, bool at_head);
void			runq_remove(t_runq *rq, t_kthread *kt);
int32_t			runq_top(const t_runq *rq);
t_kthread		*runq_pop_top(t_runq *rq);
int32_t			policy_prio(const t_kthread *kt);
void			policy_wake_boost(t_kthread *kt);
bool			policy_should_preempt(const t_kthread *cur,
					const t_kthread *woken);
void			policy_account(t_kthread *cur, uint64_t *slice_start,
					uint64_t now);
int32_t			policy_age(t_runq *rq, uint64_t now);
t_kthread		*policy_pick(t_runq *rq, t_kthread *prev, int why,
					t_kthread *idle);
bool			policy_tick(t_runq *rq, t_kthread *cur, uint64_t now,
					uint64_t *slice_start);
void			policy_requeue(t_runq *rq, t_kthread *kt, int why);
void			arch_switch_to(t_cpusched *cs, t_kthread *prev,
					t_kthread *next);
void			arch_thread_setup(t_kthread *kt);
void			sched_switch_stacks(uint64_t *save_rsp, uint64_t next_rsp);
void			sched_thread_trampoline(void);
_Noreturn void	arch_enter_user(uint64_t entry, uint64_t sp, uint64_t arg);
void			arch_idle_wait(void);
uint64_t		arch_irqs_enabled(void);
uint64_t		arch_boot_stack_bottom(void);
uint64_t		arch_boot_stack_top(void);
int				sched_selftest(void);
int				st_fair(void);
int				st_preempt(void);
int				st_pingpong(void);
int				st_sleep(void);
int				st_mutex(void);
int				st_join(void);
int				st_churn(void);
int				st_failpath(void);
int				st_spawn(t_threadfn fn, void *arg, int32_t prio,
					t_thread **out);

#endif
