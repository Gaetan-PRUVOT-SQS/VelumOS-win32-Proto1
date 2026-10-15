#include "th.h"
#include "velum/err.h"

t_handle	th_open(uint32_t kind, t_reader **r)
{
	t_handle	h;
	int			rc;

	fproc_set(PF_INPUT);
	h = 0;
	rc = input_open_kind(proc_current(), kind, &h);
	if (rc < 0)
		return (0);
	if (r)
		*r = (t_reader *)g_fobj.slot[h - 1]->impl;
	return (h);
}

void	th_push_n(uint32_t type, uint32_t first, uint32_t count)
{
	t_inpevent	ev;
	uint32_t	i;

	i = 0;
	while (i < count)
	{
		ev = th_event(type, first + i);
		input_push(&ev);
		i++;
	}
}

int	th_take_codes(t_reader *r, uint32_t *codes, uint32_t max)
{
	t_inpevent	ev[INPQ_TAKE_CHUNK];
	uint32_t	n;
	uint32_t	i;
	int			total;

	total = 0;
	n = reader_take(r, ev, INPQ_TAKE_CHUNK);
	while (n > 0)
	{
		i = 0;
		while (i < n && (uint32_t)total < max)
			codes[total++] = ev[i++].code;
		n = reader_take(r, ev, INPQ_TAKE_CHUNK);
	}
	return (total);
}

static void	dummy_destroy(t_object *obj)
{
	(void)obj;
}

t_handle	th_other_handle(void)
{
	static const t_objops	ops = {"dummy", dummy_destroy, NULL};
	t_object				*o;
	t_handle				h;

	o = obj_create(OBJ_EVENT, &ops, NULL);
	if (!o || handle_alloc(proc_current(), o, HR_ALL, &h) < 0)
		return (0);
	obj_unref(o);
	return (h);
}
