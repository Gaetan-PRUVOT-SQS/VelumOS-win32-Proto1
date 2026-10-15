#include "proc_sys.h"
#include "velum/err.h"
#include "velum/klog.h"

static t_systab	g_systab;

int	syscall_register(uint32_t num, t_sysfn fn, const char *name)
{
	t_sysfn	expected;

	if (num >= SYS_MAX || !fn)
		return (E_INVAL);
	expected = NULL;
	if (!__atomic_compare_exchange_n(&g_systab.fn[num], &expected, fn, 0,
			__ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
		return (E_EXIST);
	__atomic_store_n(&g_systab.name[num], name, __ATOMIC_RELEASE);
	return (0);
}

static int64_t	syscall_unknown(uint64_t num)
{
	uint64_t	n;

	n = __atomic_add_fetch(&g_systab.unknown, 1, __ATOMIC_RELAXED);
	if (n < SYSTAB_LOG_MAX)
		klog_warn("syscall: appel inconnu %#llx", (unsigned long long)num);
	else if (n == SYSTAB_LOG_MAX)
		klog_warn("syscall: appel inconnu %#llx, journal coupé",
			(unsigned long long)num);
	return (E_NOSYS);
}

int64_t	syscall_dispatch(t_sysargs *args, uint32_t num)
{
	t_sysfn		fn;
	uint32_t	idx;

	idx = num & (SYSTAB_SIZE - 1);
	if (idx != num)
		return (syscall_unknown(num));
	__asm__ volatile ("lfence" : : : "memory");
	fn = __atomic_load_n(&g_systab.fn[idx], __ATOMIC_ACQUIRE);
	if (!fn)
		return (syscall_unknown(num));
	__atomic_add_fetch(&g_systab.calls[idx], 1, __ATOMIC_RELAXED);
	return (fn(args));
}

uint64_t	syscall_calls(uint32_t num)
{
	if (num >= SYSTAB_SIZE)
		return (0);
	return (__atomic_load_n(&g_systab.calls[num], __ATOMIC_RELAXED));
}

uint64_t	syscall_unknown_count(void)
{
	return (__atomic_load_n(&g_systab.unknown, __ATOMIC_RELAXED));
}
