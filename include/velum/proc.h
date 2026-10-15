#ifndef PROC_H
# define PROC_H

# include <stdint.h>
# include "abi/abi_syscall.h"
# include "abi/abi_types.h"
# include "cpu.h"
# include "sched.h"
# include "sync.h"
# include "vmm.h"

# define PS_RUNNING 0
# define PS_EXITING 1
# define PS_ZOMBIE 2
# define PROC_ARGS_MAX 32768

typedef struct s_process
{
	uint32_t				pid;
	uint32_t				ppid;
	char					name[32];
	uint32_t				flags;
	uint32_t				state;
	int32_t					exit_code;
	uint32_t				nthreads;
	uint32_t				refs;
	t_aspace				*aspace;
	struct s_handle_table	*handles;
	t_thread				*threads;
	struct s_object			*obj;
	uint64_t				created_ns;
	uint64_t				mem_limit;
	uint64_t				image_base;
	uint64_t				entry;
	t_spinlock				lock;
}	t_process;

typedef struct s_spawnreq
{
	const char			*path;
	const void			*image;
	uint64_t			image_size;
	const char			*args;
	uint64_t			args_len;
	uint32_t			flags;
	t_process			*parent;
}	t_spawnreq;

int				proc_boot_init(void);
t_process		*proc_current(void);
t_process		*proc_find(uint32_t pid);
int				proc_spawn(const t_spawnreq *rq, t_process **out);
void			proc_ref(t_process *p);
void			proc_unref(t_process *p);
void			proc_kill(t_process *p, int code);
void			proc_fault(t_regs *regs, uint64_t vec, uint64_t addr);
int				proc_list(t_procinfo *out, uint32_t max);
int				init_start(void);
int				proc_selftest(void);
void			proc_return_check(void);
_Noreturn void	proc_exit_current(int code);
void			object_process_cleanup(t_process *p);
int				proc_count(void);

#endif
