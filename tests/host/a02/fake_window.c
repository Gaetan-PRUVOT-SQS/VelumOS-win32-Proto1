#include <string.h>
#include <sys/mman.h>
#include "a02_fake.h"

static void		*g_window;
static size_t	g_window_len;

int	fake_window_map(uint64_t top)
{
	g_window_len = align_up(max_u64(top, PAGE_SIZE), PAGE_SIZE);
	g_window = mmap(NULL, g_window_len, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
	if (g_window == MAP_FAILED)
	{
		g_window = NULL;
		return (-1);
	}
	g_fake_info.hhdm = (uint64_t)(uintptr_t)g_window;
	return (0);
}

void	fake_window_poison(void)
{
	size_t	len;

	len = min_u64(g_window_len, 48 * MIB);
	memset((char *)g_window + g_window_len - len, FAKE_POISON, len);
}

void	fake_window_drop(void)
{
	if (g_window)
		munmap(g_window, g_window_len);
	g_window = NULL;
	g_window_len = 0;
}
