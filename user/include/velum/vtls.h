#ifndef VTLS_H
# define VTLS_H

# include <stdint.h>

# define V_TCB_OFF_SELF 0
# define V_TCB_OFF_ERRNO 8
# define V_TCB_OFF_TID 12
# define V_TCB_OFF_THREAD 16

typedef struct s_vtcb
{
	struct s_vtcb	*self;
	int32_t			err;
	uint32_t		tid;
	void			*thread;
	uint64_t		reserved[4];
}	t_vtcb;

t_vtcb	*v_tcb(void);
int		*__velum_errno(void);

#endif
