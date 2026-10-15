#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "d09.h"

t_fk	g_fk;

static void	load(const char *name)
{
	char	path[512];
	FILE	*f;
	size_t	n;

	snprintf(path, sizeof(path), "%s/%s", D09_FIX, name);
	f = fopen(path, "rb");
	g_fk.file = malloc(1 << 20);
	n = 0;
	if (f && g_fk.file)
		n = fread(g_fk.file, 1, 1 << 20, f);
	if (f)
		fclose(f);
	h_eq_i64("dex_open", dex_open(&g_fk.dex, (t_span){g_fk.file, n}), 0);
}

void	fk_init(const char *dex, uint32_t depth, uint64_t budget)
{
	memset(&g_fk, 0, sizeof(g_fk));
	g_fk.vm.stack = calloc(FK_STACK, sizeof(uint32_t));
	g_fk.vm.stack_words = FK_STACK;
	g_fk.vm.lim.stack_words = FK_STACK;
	g_fk.vm.lim.depth_max = depth;
	g_fk.vm.lim.budget = budget;
	if (dex)
		load(dex);
}

void	fk_end(void)
{
	uint32_t	k;

	k = 0;
	while (k < FK_OBJS)
		free(g_fk.obj[k++].data);
	free(g_fk.vm.stack);
	free(g_fk.file);
}

void	fk_check(const char *what)
{
	h_eq_u64(what, g_fk.vm.sp, 0);
	h_eq_u64(what, g_fk.vm.depth, 0);
}

t_dref	fk_obj(const char *desc, uint32_t slots)
{
	t_fkobj	*o;

	if (g_fk.nobj + 1 >= FK_OBJS)
		return (DVM_NULL);
	g_fk.nobj++;
	o = &g_fk.obj[g_fk.nobj];
	o->desc = desc;
	o->len = slots;
	o->data = calloc(slots + 1, sizeof(uint64_t));
	return (g_fk.nobj);
}
