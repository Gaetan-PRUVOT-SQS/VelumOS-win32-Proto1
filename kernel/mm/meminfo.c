#include "heap_int.h"
#include "meminfo.h"
#include "velum/err.h"
#include "velum/ksyscall.h"
#include "velum/vmm.h"

extern int	syscall_register(uint32_t num, t_sysfn fn,
				const char *name) __attribute__((weak));

static uint64_t	heap_bytes_live(void)
{
	t_heap_stats	st;
	uint64_t		sum;
	uint32_t		i;

	heap_get_stats(&st);
	sum = 0;
	i = 0;
	while (i < HEAP_TAGS)
	{
		sum += st.bytes_live[i];
		i++;
	}
	return (sum);
}

void	meminfo_collect(t_meminfo *out)
{
	t_pmm_stats	ps;

	pmm_get_stats(&ps);
	meminfo_build(out, &ps, heap_window_bytes(&g_heap.lay),
		heap_bytes_live());
}

static int64_t	sys_meminfo(const t_sysargs *a)
{
	t_meminfo	mi;
	int			rc;

	meminfo_collect(&mi);
	rc = copy_to_user((t_uptr)a->a[0], &mi, sizeof(mi));
	if (rc < 0)
		return (rc);
	return (0);
}

int	meminfo_register(void)
{
	if (!syscall_register)
		return (E_NOSYS);
	return (syscall_register(SYS_MEMINFO, sys_meminfo, "meminfo"));
}
