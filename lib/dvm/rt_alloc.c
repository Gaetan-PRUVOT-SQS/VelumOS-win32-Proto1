#include "rt_int.h"

static int	rt_fits(const struct s_dheap *h, uint64_t bytes)
{
	return (bytes <= h->limit && h->st.bytes <= h->limit - bytes);
}

static int	rt_room(t_dvm *vm, uint64_t bytes)
{
	if (rt_fits(vm->heap, bytes))
		return (0);
	dvm_gc(vm);
	if (rt_fits(vm->heap, bytes))
		return (0);
	if (!rt_obj(vm, vm->heap->oom, DOK_OBJECT))
		return (E_NOMEM);
	vm->pending = vm->heap->oom;
	return (DVM_THROWN);
}

static void	rt_fill(struct s_dheap *h, uint32_t i, const t_dnewobj *rq,
		void *data)
{
	t_dobj	*o;

	o = &h->tab[i];
	h->free_head = o->next;
	o->cls = rq->cls;
	o->data = data;
	o->len = rq->len;
	o->bytes = (uint32_t)rq->bytes + DOBJ_COST;
	o->kind = rq->kind;
	o->pins = 0;
	o->mark = 0;
	h->st.bytes += o->bytes;
	h->st.objects++;
	if (h->st.bytes > h->st.peak_bytes)
		h->st.peak_bytes = h->st.bytes;
}

int	rt_alloc(t_dvm *vm, const t_dnewobj *rq, t_dref *out)
{
	struct s_dheap	*h;
	void			*data;
	int				r;

	h = vm->heap;
	*out = DVM_NULL;
	r = rt_room(vm, rq->bytes + DOBJ_COST);
	if (r == 0 && h->free_head == 0)
		r = rt_grow(h);
	if (r != 0)
		return (r);
	data = calloc(1, (size_t)rq->bytes + 8u);
	if (!data)
		return (E_NOMEM);
	*out = h->free_head;
	rt_fill(h, h->free_head, rq, data);
	return (0);
}

void	dvm_heap_stats(const t_dvm *vm, t_dheapstats *out)
{
	if (!vm || !vm->heap || !out)
		return ;
	*out = vm->heap->st;
}
