#include <stdlib.h>
#include <string.h>
#include "a07_fake.h"

const uint8_t	g_ust_rnd[USTACK_RANDOM] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
	11, 12, 13, 14, 15, 16};

void	ust_input(t_ustart *in, const char *args, uint64_t len, int n)
{
	memset(in, 0, sizeof(*in));
	in->path = "/system/bin/init";
	in->path_len = strlen(in->path);
	in->args = args;
	in->args_len = len;
	in->nargs = (uint32_t)n;
	in->flags = 0x7f;
	in->rnd = g_ust_rnd;
	in->entry = 0x10001000;
	in->phdr = 0x10000040;
	in->phnum = 6;
}

int	ust_aux_order_ok(const t_ustparsed *p)
{
	const uint64_t	types[8] = {AT_PHDR, AT_PHNUM, AT_PAGESZ, AT_ENTRY,
		AT_RANDOM, AT_VELUM_ABI, AT_VELUM_FLAGS, AT_NULL};
	int				i;

	if (p->naux != 8)
		return (0);
	i = 0;
	while (i < 8)
	{
		if (p->aux_type[i] != types[i])
			return (0);
		i++;
	}
	return (1);
}

char	*args_block(int n, uint64_t each)
{
	char	*b;
	int		i;

	b = malloc((uint64_t)n * each + 1);
	if (!b)
		abort();
	memset(b, 'x', (uint64_t)n * each + 1);
	i = 1;
	while (i <= n)
	{
		b[(uint64_t)i * each - 1] = '\0';
		i++;
	}
	return (b);
}
