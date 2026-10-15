#ifndef PROC_INT_H
# define PROC_INT_H

# include <stdint.h>
# include "proc_deps.h"
# include "proc_elf.h"
# include "proc_pure.h"
# include "velum/abi/abi_types.h"

# define PROC_MAX 256
# define PROC_THREADS_MAX 64
# define PROC_MEM_DEFAULT 0x4000000ull
# define USTACK_SIZE 0x100000ull
# define USTACK_SLOT 0x200000ull
# define USTACK_INIT_MAX 0xa000ull
# define TSTACK_DEFAULT 0x40000ull
# define TSTACK_MIN 0x4000ull
# define TSTACK_MAX 0x800000ull
# define PROC_HRIGHTS 0x187
# define THREAD_HRIGHTS 0x185
# define PT_FREE 0
# define PT_RESERVED 1
# define PT_LIVE 2
# define PT_DEAD 3
# define PT_RECLAIM 4
# define BOOT_ARG_MAX 63
# define BOOT_NARGS_MAX 8
# define BOOT_ARGS_CAP 256

typedef struct s_pthread
{
	t_thread	*t;
	uint64_t	stack_va;
	uint64_t	stack_len;
	uint32_t	state;
	uint32_t	reserved;
}	t_pthread;

typedef struct s_procext
{
	t_ratelimit	rl;
	t_pthread	th[PROC_THREADS_MAX];
}	t_procext;

typedef struct s_procbox
{
	t_process	p;
	t_procext	ext;
}	t_procbox;

typedef struct s_proctab
{
	t_spinlock	lock;
	t_process	*slot[PROC_MAX];
	uint32_t	next_pid;
	uint32_t	count;
	uint32_t	ready;
	uint32_t	init_pid;
}	t_proctab;

typedef struct s_reapq
{
	t_spinlock	lock;
	t_process	*q[PROC_MAX];
	uint32_t	head;
	uint32_t	n;
	t_event		ev;
	t_thread	*thread;
}	t_reapq;

typedef struct s_spawnctx
{
	const t_spawnreq	*rq;
	const void			*img;
	uint64_t			size;
	void				*owned;
	t_process			*p;
	t_handle			*hout;
	t_process			**out;
	uint64_t			base;
	uint64_t			sp;
	uint64_t			path_len;
	int32_t				nargs;
	uint32_t			flags;
	char				path[VFS_PATH_MAX];
	t_elfinfo			info;
}	t_spawnctx;

t_proctab		*proc_tab(void);
t_procext		*proc_ext(t_process *p);
int				proc_insert(t_process *p);
void			proc_remove(t_process *p);
void			proc_free(t_process *p);
void			proc_fill_info(t_process *p, t_procinfo *out);
int				proc_spawn_handle(const t_spawnreq *rq, t_handle *h);
int				spawn_run(t_spawnctx *c);
int				proc_load_image(t_spawnctx *c);
int				aspace_write(t_aspace *as, uint64_t va, const void *src,
					uint64_t n);
int				proc_setup_stack(t_spawnctx *c);
uint64_t		proc_random_slot(uint64_t n);
int				proc_start_main(t_spawnctx *c);
void			proc_abort(t_process *p);
int				proc_make_obj(t_process *p);
void			*proc_handle_obj(t_process *p, uint64_t h, uint32_t type,
					t_hget *out);
void			proc_obj_release(t_hget *g);
void			proc_thread_release(t_thread *t);
t_thread		*proc_thread_add(t_process *p, const t_threadreq *rq,
					const t_pthread *st);
void			proc_reclaim(t_process *p);
int				proc_tstack_alloc(t_process *p, uint64_t size, uint64_t arg,
					t_pthread *st);
void			proc_mark_exiting(t_process *p, int code);
void			proc_wake_threads(t_process *p);
void			proc_thread_leave(t_process *p, t_thread *t, int code);
int				reap_init(void);
void			reap_enqueue(t_process *p);
void			proc_finish(t_process *p);
int				syscall_prio_check(uint64_t prio);
int				proc_stack_map(t_aspace *as, uint64_t top);
int				proc_spawn_selftest(void);
uint32_t		boot_args_init(const char *cmdline, char *out,
					uint32_t cap);

#endif
