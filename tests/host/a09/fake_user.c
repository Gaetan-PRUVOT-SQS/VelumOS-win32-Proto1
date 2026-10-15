#include <stdlib.h>
#include <string.h>
#include "a09_test.h"
#include "fake.h"

void	fake_user_map(uint64_t base, size_t len, int fill)
{
	g_fake.user_mem = malloc(len);
	g_fake.user_base = base;
	g_fake.user_len = len;
	memset(g_fake.user_mem, fill, len);
}

void	fake_user_unmap(void)
{
	free(g_fake.user_mem);
	g_fake.user_mem = NULL;
}

int	bytes_are(const uint8_t *mem, size_t from, size_t to, uint8_t value)
{
	while (from < to)
	{
		if (mem[from] != value)
			return (0);
		from++;
	}
	return (1);
}

int	tail_is_zero(const char *field, size_t size)
{
	return (bytes_are((const uint8_t *)field, strlen(field), size, 0));
}
