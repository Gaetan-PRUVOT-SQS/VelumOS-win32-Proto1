#include "fake.h"
#include "velum/err.h"

int	handle_alloc(t_process *p, t_object *o, uint32_t rt, t_handle *out)
{
	t_fakeh		*h;
	uint32_t	i;

	(void)p;
	h = fake_htab();
	i = 0;
	while (i < FAKE_HANDLES && h[i].used)
		i++;
	if (i == FAKE_HANDLES)
		return (E_MFILE);
	h[i].used = 1;
	h[i].obj = o;
	h[i].rights = rt;
	o->refs++;
	*out = i + 1;
	return (0);
}

int	handle_get(t_process *p, t_handle hv, uint32_t type, t_hget *out)
{
	t_fakeh	*h;

	(void)p;
	h = fake_htab();
	if (hv == 0 || hv > FAKE_HANDLES || !h[hv - 1].used)
		return (E_BADF);
	if (h[hv - 1].obj->type != type)
		return (E_BADF);
	h[hv - 1].obj->refs++;
	out->obj = h[hv - 1].obj;
	out->rights = h[hv - 1].rights;
	return (0);
}

int	fake_close(t_handle hv)
{
	t_fakeh	*h;

	h = fake_htab();
	if (hv == 0 || hv > FAKE_HANDLES || !h[hv - 1].used)
		return (E_BADF);
	h[hv - 1].used = 0;
	obj_unref(h[hv - 1].obj);
	return (0);
}

const t_sysargs	*fake_args(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3)
{
	static t_sysargs	args;

	args.a[0] = a0;
	args.a[1] = a1;
	args.a[2] = a2;
	args.a[3] = a3;
	args.a[4] = 0;
	args.a[5] = 0;
	args.regs = NULL;
	return (&args);
}
