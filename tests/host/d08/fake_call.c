#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rt_int.h"
#include "d08.h"

t_d08call	g_d08call;

int	dvm_call(t_dvm *vm, const t_dmethod *m, const uint32_t *args,
		uint64_t *ret)
{
	(void)vm;
	(void)args;
	(void)ret;
	g_d08call.calls++;
	g_d08call.last = m;
	return (0);
}

int	d08_open(t_dvm **vm, t_span *file)
{
	FILE	*f;
	uint8_t	*buf;

	g_d08call.calls = 0;
	f = fopen(D08_FIX "/essai.dex", "rb");
	buf = malloc(1 << 16);
	if (!f || !buf)
		return (-1);
	file->len = fread(buf, 1, 1 << 16, f);
	file->p = buf;
	fclose(f);
	if (dvm_create(vm, NULL) != 0)
		return (-1);
	return (dvm_load_dex(*vm, *file));
}

void	d08_close(t_dvm *vm, t_span file)
{
	dvm_destroy(vm);
	free((void *)(uintptr_t)file.p);
}

int	d08_method_idx(t_dvm *vm, const char *cls, const char *name)
{
	const t_dex	*d;
	t_dexmethod	m;
	uint32_t	i;

	d = &vm->classes->dex;
	i = 0;
	while (i < d->n[DEX_T_METHOD] && dex_method(d, i, &m) == 0)
	{
		if (!strcmp(rt_dtype(d, m.class_idx), cls)
			&& !strcmp(rt_dstr(d, m.name_idx), name))
			return ((int)i);
		i++;
	}
	return (-1);
}

int	d08_field_idx(t_dvm *vm, const char *cls, const char *name)
{
	const t_dex	*d;
	t_dexfield	f;
	uint32_t	i;

	d = &vm->classes->dex;
	i = 0;
	while (i < d->n[DEX_T_FIELD] && dex_field(d, i, &f) == 0)
	{
		if (!strcmp(rt_dtype(d, f.class_idx), cls)
			&& !strcmp(rt_dstr(d, f.name_idx), name))
			return ((int)i);
		i++;
	}
	return (-1);
}
