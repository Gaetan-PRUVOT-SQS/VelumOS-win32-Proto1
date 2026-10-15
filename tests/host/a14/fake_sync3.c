#include <unistd.h>
#include "errno.h"
#include "fake_sync.h"
#include "fake_sys.h"
#include "velum/vtime.h"

void	*fs_ret_plus1(void *arg)
{
	return ((void *)((uintptr_t)arg + 1));
}

void	*fs_self_check(void *arg)
{
	t_vthread	*me;

	me = v_thread_self();
	return ((void *)(uintptr_t)(me != NULL && me->arg == arg
		&& me->fn == fs_self_check));
}

void	*fs_errno_check(void *arg)
{
	int	mine;

	mine = (int)(uintptr_t)arg;
	errno = mine;
	v_yield();
	usleep(1000);
	return ((void *)(uintptr_t)(errno == mine));
}

void	*fs_exit_inside(void *arg)
{
	v_thread_exit((void *)((uintptr_t)arg * 2));
}

void	*fs_join_self(void *arg)
{
	*(int *)arg = v_thread_join(v_thread_self(), NULL);
	return (NULL);
}
