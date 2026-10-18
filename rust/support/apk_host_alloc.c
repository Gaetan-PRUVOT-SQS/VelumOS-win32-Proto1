#include <stddef.h>
#include <stdlib.h>

void *d07_malloc(size_t n) __attribute__((weak));

void *apk_host_alloc(size_t n)
{
	void *(*fn)(size_t);

	fn = d07_malloc;
	if (fn != 0)
		return (fn(n));
	return (malloc(n));
}

void apk_host_free(void *p)
{
	free(p);
}
