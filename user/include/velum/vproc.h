#ifndef VPROC_H
# define VPROC_H

# include <stddef.h>
# include <stdint.h>
# include "velum/abi/abi_syscall.h"
# include "velum/abi/abi_types.h"

# define V_ARGS_MAX 32768
# define V_ARGC_MAX 256
# define V_PRIO_IDLE 0
# define V_PRIO_LOW 4
# define V_PRIO_NORMAL 8
# define V_PRIO_HIGH 13
# define V_PRIO_REALTIME 24
# define V_PRIO_MAX 31

# ifndef PS_RUNNING
#  define PS_RUNNING 0
#  define PS_EXITING 1
#  define PS_ZOMBIE 2
# endif

typedef struct s_vthreadreq
{
	uint64_t	entry;
	uint64_t	arg;
	uint64_t	stack_size;
	uint32_t	prio;
	uint32_t	flags;
}	t_vthreadreq;

_Noreturn void	v_exit(int code);
int64_t			v_spawn(const char *path, const char *args, size_t args_len,
					uint32_t flags);
int64_t			v_args_pack(char *buf, size_t cap, const char *const *argv);
int64_t			v_spawnv(const char *path, const char *const *argv,
					uint32_t flags);
int				v_proc_kill(t_handle proc, int code);
int				v_proc_info(t_handle proc, t_procinfo *out);
int64_t			v_thread_create_raw(const t_vthreadreq *req);
_Noreturn void	v_thread_exit_raw(int code);
int				v_thread_prio(t_handle thread, uint32_t prio);
int64_t			v_gettid(void);
int				v_set_fsbase(uint64_t base);
int				v_proc_list(t_procinfo *out, uint32_t max);

#endif
