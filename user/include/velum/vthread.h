#ifndef VTHREAD_H
# define VTHREAD_H

# include <stdint.h>
# include "velum/vsync.h"
# include "velum/vtls.h"

# define V_THREAD_STACK_DEFAULT 262144
# define V_THREAD_STACK_MIN 16384
# define V_THREAD_STACK_MAX 8388608
# define V_THREAD_RUNNING 0
# define V_THREAD_FINISHED 1
# define V_THREAD_DETACHED 2
# define V_THREAD_JOINED 3

typedef void	*(*t_vthreadfn)(void *);

typedef struct s_vthread
{
	t_vtcb		tcb;
	t_vthreadfn	fn;
	void		*arg;
	void		*result;
	t_vevent	done;
	uint32_t	handle;
	uint32_t	state;
	uint32_t	joining;
	uint32_t	reserved;
}	t_vthread;

typedef struct s_vthreadattr
{
	uint64_t	stack_size;
	uint32_t	prio;
	uint32_t	flags;
}	t_vthreadattr;

int				v_thread_create(t_vthread **out, t_vthreadfn fn, void *arg);
int				v_thread_create_ex(t_vthread **out, t_vthreadfn fn, void *arg,
					const t_vthreadattr *attr);
int				v_thread_join(t_vthread *thread, void **result);
int				v_thread_detach(t_vthread *thread);
t_vthread		*v_thread_self(void);
_Noreturn void	v_thread_exit(void *result);

#endif
