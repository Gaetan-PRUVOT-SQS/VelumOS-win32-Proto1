#ifndef FAKE_H
# define FAKE_H

# include <stdint.h>
# include "obj_int.h"
# include "velum/heap.h"
# include "velum/pmm.h"
# include "velum/util.h"

# define FK_MAPS 4096
# define FK_VMQ_ON_FAIL 1
# define FK_SYS 256
# define FK_TIMERS 64
# define FK_VA_BASE 0x10000000
# define FK_RACES 100000
# define FK_SIM_OBJS 6

typedef int		(*t_fkhook)(void *ctx, int forced);
typedef void	(*t_fkvmq)(void);

typedef struct s_fkmap
{
	void		*as;
	uint64_t	va;
	uint64_t	pa;
	uint32_t	flags;
	uint32_t	used;
}	t_fkmap;

typedef struct s_fktimer
{
	uint64_t	deadline;
	t_timerfn	fn;
	void		*ctx;
	int64_t		id;
	int			armed;
	int			pad;
}	t_fktimer;

typedef struct s_fake
{
	int64_t		heap_fail;
	int64_t		heap_live[HEAP_TAGS];
	int64_t		pmm_fail;
	int64_t		pmm_live;
	int64_t		vmm_fail;
	uint64_t	now;
	int			depth;
	int			in_hook;
	t_fkhook	hook;
	void		*hook_ctx;
	int			wait_end_rc;
	int			slept_at_end;
	uint64_t	waits;
	t_process	*cur;
	t_fkmap		maps[FK_MAPS];
	uint64_t	next_va;
	uint64_t	pages_used;
	t_sysfn		sys[FK_SYS];
	t_fktimer	timers[FK_TIMERS];
	int64_t		next_tid;
	uint64_t	fault_lo;
	uint64_t	fault_hi;
	int64_t		proc_refs;
	int64_t		thread_refs;
	uint64_t	rng;
	t_fkvmq		vmq_hook;
	uint64_t	vmq_va;
	int			vmq_pre;
	int			vmq_late;
	int64_t		vmq_h;
	int64_t		vmq_n;
	int64_t		unmap_fail;
}	t_fake;

typedef struct s_sim
{
	t_object	*objs[FK_SIM_OBJS];
	int			ever[FK_SIM_OBJS];
	uint32_t	n;
	uint32_t	mode;
	int			budget;
	int			lost;
	int			ok;
	int			cancels;
}	t_sim;

extern t_fake	g_fk;

void		fk_reset(void);
int64_t		fk_call(uint32_t num, uint64_t a0, uint64_t a1, uint64_t a2);
int64_t		fk_call6(uint32_t num, const uint64_t *args);
void		fk_fire_due(void);
t_process	*fk_proc_new(uint32_t flags);
void		fk_proc_free(t_process *p);
int64_t		fk_heap_total(void);
uint64_t	fk_rand(void);
void		fk_seed(uint64_t seed);
uint32_t	fk_count_maps(void *as);
uint64_t	fk_uptr(const void *p);
int			fk_hook_run(int forced);
void		fk_chan_pair(t_process *pa, t_process *pb, t_handle *out);
t_handle	fk_event_in(t_process *p, uint32_t rights);
int64_t		fk_send(t_handle h, const t_chansend *cs);
int64_t		fk_recv(t_handle h, t_chanrecv *cr);
t_object	*fk_obj(t_process *p, t_handle h);
int64_t		fk_map(int64_t h, uint64_t hint, uint64_t prot, uint64_t off);
void		fk_vmq_fire(uint64_t page, int pre);
void		fk_vmq_arm(t_fkvmq fn, uint64_t va, int pre);
int			fk_vmm_refuse(void);

#endif
