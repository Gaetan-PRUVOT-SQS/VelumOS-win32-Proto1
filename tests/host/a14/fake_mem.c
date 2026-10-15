#include <pthread.h>
#include <sys/mman.h>
#include "fake_int.h"
#include "fake_sys.h"
#include "velum/err.h"

static t_fmap			g_fmaps[FAKE_MAPS];
static pthread_mutex_t	g_fmem_lock = PTHREAD_MUTEX_INITIALIZER;

static int	fake_map_find(uint64_t base, uint64_t len)
{
	int	i;

	i = 0;
	while (i < FAKE_MAPS)
	{
		if (g_fmaps[i].len && g_fmaps[i].base == base && g_fmaps[i].len == len)
			return (i);
		i++;
	}
	return (-1);
}

static void	*fake_map_hint(uint64_t len, int *flags)
{
	void	*hint;

	hint = NULL;
	*flags = MAP_PRIVATE | MAP_ANONYMOUS;
	if (g_fsys.valloc_reuse && g_fsys.last_freed
		&& len <= g_fsys.last_freed_len)
	{
		hint = (void *)(uintptr_t)g_fsys.last_freed;
		*flags |= MAP_FIXED;
	}
	g_fsys.last_freed = 0;
	return (hint);
}

static int64_t	fake_map_new(uint64_t len)
{
	void	*p;
	void	*hint;
	int		i;
	int		flags;

	hint = fake_map_hint(len, &flags);
	p = mmap(hint, len, PROT_READ | PROT_WRITE, flags, -1, 0);
	if (p == MAP_FAILED)
		return (E_NOMEM);
	i = 0;
	while (i < FAKE_MAPS && g_fmaps[i].len)
		i++;
	if (i == FAKE_MAPS)
	{
		munmap(p, len);
		return (E_NOMEM);
	}
	g_fmaps[i].base = (uint64_t)(uintptr_t)p;
	g_fmaps[i].len = len;
	g_fsys.valloc_calls++;
	g_fsys.live_maps++;
	g_fsys.live_bytes += len;
	return ((int64_t)(uintptr_t)p);
}

int64_t	fake_valloc(const uint64_t *a)
{
	int64_t	r;

	if (!a[1] || (a[1] & 4095))
		return (E_INVAL);
	pthread_mutex_lock(&g_fmem_lock);
	r = E_NOMEM;
	if (g_fsys.valloc_budget != 0)
	{
		r = fake_map_new(a[1]);
		if (r > 0 && g_fsys.valloc_budget > 0)
			g_fsys.valloc_budget--;
	}
	pthread_mutex_unlock(&g_fmem_lock);
	return (r);
}

int64_t	fake_vfree(const uint64_t *a)
{
	int	i;

	pthread_mutex_lock(&g_fmem_lock);
	i = fake_map_find(a[0], a[1]);
	if (i < 0)
	{
		g_fsys.vfree_bad++;
		pthread_mutex_unlock(&g_fmem_lock);
		return (E_INVAL);
	}
	munmap((void *)(uintptr_t)a[0], a[1]);
	g_fsys.last_freed = a[0];
	g_fsys.last_freed_len = a[1];
	g_fmaps[i].len = 0;
	g_fsys.vfree_calls++;
	g_fsys.live_maps--;
	g_fsys.live_bytes -= a[1];
	pthread_mutex_unlock(&g_fmem_lock);
	return (0);
}
