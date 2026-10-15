#include <sys/mman.h>
#include "fake_sys.h"
#include "velum/util.h"

void	*fake_guarded(size_t bytes)
{
	size_t	body;
	char	*base;

	body = align_up(bytes, PAGE_SIZE);
	if (!body)
		body = PAGE_SIZE;
	base = mmap(NULL, body + PAGE_SIZE, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (base == MAP_FAILED)
		return (NULL);
	mprotect(base + body, PAGE_SIZE, PROT_NONE);
	return (base + body - bytes);
}

void	fake_guarded_free(void *p, size_t bytes)
{
	size_t	body;
	char	*base;

	body = align_up(bytes, PAGE_SIZE);
	if (!body)
		body = PAGE_SIZE;
	base = (char *)p + bytes - body;
	munmap(base, body + PAGE_SIZE);
}
