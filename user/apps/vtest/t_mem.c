#include "stdlib.h"
#include "string.h"
#include "vtest.h"

static void	mem_blocks(void)
{
	static void	*p[500];
	size_t		n;
	int			i;
	int			ok;

	ok = 1;
	i = -1;
	while (++i < 500)
	{
		n = (size_t)(i * 37 % 900) + 1;
		p[i] = malloc(n);
		ok = ok && p[i] != NULL;
		if (p[i])
			memset(p[i], i, n);
	}
	while (i--)
	{
		n = (size_t)(i * 37 % 900) + 1;
		ok = ok && ((uint8_t *)p[i])[n - 1] == (uint8_t)i;
		free(p[i]);
	}
	vtest_check(ok, "mem: 500 blocs de tailles variees");
}

static void	mem_large(void)
{
	uint8_t	*p;
	uint8_t	*q;

	p = malloc(1 << 20);
	vtest_check(p != NULL, "mem: bloc de 1 Mio");
	memset(p, 0x5a, 1 << 20);
	q = realloc(p, 2 << 20);
	vtest_check(q != NULL && q[0] == 0x5a && q[(1 << 20) - 1] == 0x5a,
		"mem: realloc d'un gros bloc conserve le contenu");
	free(q);
	vtest_check(malloc((size_t)-1) == NULL, "mem: taille absurde refusee");
}

static void	mem_calloc(void)
{
	uint8_t	*p;
	int		i;
	int		zero;

	p = calloc(100, 3);
	zero = p != NULL;
	i = 0;
	while (p && i < 300)
		zero = zero && p[i++] == 0;
	vtest_check(zero, "mem: calloc remet a zero");
	free(p);
	vtest_check(calloc((size_t)-1 / 2, 3) == NULL, "mem: calloc deborde");
}

static void	mem_stats(void)
{
	t_vheapstats	before;
	t_vheapstats	after;
	void			*p;

	v_heap_stats(&before);
	p = malloc(777);
	free(p);
	p = malloc(100000);
	free(p);
	v_heap_stats(&after);
	vtest_check(before.live_blocks == after.live_blocks,
		"mem: blocs vivants stables");
	vtest_check(after.faults == 0, "mem: aucune faute du tas");
}

void	vtest_mem(void)
{
	mem_blocks();
	mem_large();
	mem_calloc();
	mem_stats();
}
