#include <string.h>
#include "fake_int.h"
#include "fake_sys.h"
#include "velum/abi/abi_syscall.h"

static const t_fentry	g_fake_table[] = {
{SYS_VALLOC, fake_valloc}, {SYS_VFREE, fake_vfree},
{SYS_VPROTECT, fake_vprotect}, {SYS_EVENT_CREATE, fake_event_create},
{SYS_EVENT_OP, fake_event_op}, {SYS_WAIT, fake_wait},
{SYS_CLOSE, fake_close}, {SYS_THREAD_CREATE, fake_thread_create},
{SYS_THREAD_EXIT, fake_thread_exit}, {SYS_SET_FSBASE, fake_set_fsbase},
{SYS_GETTID, fake_gettid}, {SYS_LOG, fake_log},
{SYS_GETRANDOM, fake_getrandom}, {SYS_YIELD, fake_yield},
{SYS_EXIT, fake_exit}, {0xffff, NULL}
};

static t_fhandler	fake_lookup(uint64_t num)
{
	int	i;

	i = 0;
	while (g_fake_table[i].fn)
	{
		if (g_fake_table[i].num == num)
			return (g_fake_table[i].fn);
		i++;
	}
	return (NULL);
}

int64_t	v_syscall6(uint64_t num, const uint64_t *args)
{
	t_fhandler	fn;

	if (g_fsys.record)
	{
		g_fsys.last_num = num;
		memcpy(g_fsys.last_args, args, sizeof(g_fsys.last_args));
	}
	__atomic_fetch_add(&g_fsys.calls, 1, __ATOMIC_RELAXED);
	fn = fake_lookup(num);
	if (fn && g_fsys.kernel)
		return (fn(args));
	return (g_fsys.ret);
}

void	fake_reset(void)
{
	memset(&g_fsys, 0, sizeof(g_fsys));
	g_fsys.record = 1;
	g_fsys.valloc_budget = -1;
}

void	fake_kernel_on(void)
{
	g_fsys.kernel = 1;
}
