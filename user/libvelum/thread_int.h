#ifndef THREAD_INT_H
# define THREAD_INT_H

# include "velum/vthread.h"

void			__velum_thread_entry(void);
_Noreturn void	__velum_thread_main(t_vthread *thread);
void			thread_cleanup(t_vthread *thread);

#endif
