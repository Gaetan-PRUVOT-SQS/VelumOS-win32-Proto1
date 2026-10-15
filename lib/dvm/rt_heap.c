#include "rt_int.h"

t_dobj	*rt_obj(const t_dvm *vm, t_dref ref, uint8_t kind)
{
	t_dobj	*o;

	if (!vm || !vm->heap || ref == DVM_NULL || ref >= vm->heap->cap)
		return (NULL);
	o = &vm->heap->tab[ref];
	if (o->kind == DOK_FREE)
		return (NULL);
	if (kind != DOK_ANY && o->kind != kind)
		return (NULL);
	return (o);
}

uint32_t	*rt_words(const t_dobj *o)
{
	return ((uint32_t *)(void *)((uint8_t *)o->data + o->cls->pay_end));
}

static void	rt_chain(struct s_dheap *h, t_dobj *tab, uint32_t cap)
{
	uint32_t	i;

	i = 0;
	while (i < h->cap)
	{
		tab[i] = h->tab[i];
		i++;
	}
	while (i < cap)
	{
		if (i != 0)
		{
			tab[i].next = h->free_head;
			h->free_head = i;
		}
		i++;
	}
}

int	rt_grow(struct s_dheap *h)
{
	t_dobj		*tab;
	uint32_t	cap;

	cap = h->cap * 2;
	if (h->cap == 0)
		cap = DOBJ_FIRST;
	if (cap > DOBJ_MAX)
		return (E_NOMEM);
	tab = calloc(cap, sizeof(*tab));
	if (!tab)
		return (E_NOMEM);
	rt_chain(h, tab, cap);
	free(h->tab);
	h->tab = tab;
	h->cap = cap;
	return (0);
}

void	rt_release(struct s_dheap *h, uint32_t i)
{
	t_dobj	*o;

	o = &h->tab[i];
	free(o->data);
	h->st.bytes -= o->bytes;
	h->st.objects--;
	o->data = NULL;
	o->cls = NULL;
	o->len = 0;
	o->bytes = 0;
	o->pins = 0;
	o->mark = 0;
	o->kind = DOK_FREE;
	o->next = h->free_head;
	h->free_head = i;
}
