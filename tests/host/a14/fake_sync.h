#ifndef FAKE_SYNC_H
# define FAKE_SYNC_H

# include <stdint.h>
# include "velum/abi/abi_syscall.h"
# include "velum/vsync.h"
# include "velum/vthread.h"

typedef struct s_fsjob
{
	t_vmutex	*mutex;
	t_vspin		*spin;
	t_vevent	*event;
	uint64_t	*counter;
	uint32_t	loops;
	uint32_t	tid;
	int			rc;
	int			passed;
}	t_fsjob;

void	*fs_mutex_worker(void *arg);
void	*fs_spin_worker(void *arg);
void	*fs_lock_once(void *arg);
void	*fs_unlock_once(void *arg);
void	*fs_event_waiter(void *arg);
void	*fs_join_worker(void *arg);
int		fs_wait_waiters(uint64_t handle, int count);
void	*fs_gate_thread(void *arg);
void	*fs_ret_plus1(void *arg);
void	*fs_self_check(void *arg);
void	*fs_errno_check(void *arg);
void	*fs_exit_inside(void *arg);
void	*fs_join_self(void *arg);
void	*fs_tid_get(void *arg);
int		fs_wait_threads(void);
void	fs_h1(void);
void	fs_h2(void);
void	fs_h3(void);
void	fs_h_reg(void);
void	fs_h_noop(void);
void	fs_h_push(int v);
int		fs_h_count(void);
int		fs_h_at(int i);
void	fs_h_reset(void);

#endif
