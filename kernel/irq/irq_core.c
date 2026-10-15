#include "irq_int.h"
#include "velum/err.h"
#include "velum/libk.h"

void	irqc_init(t_irq_core *c)
{
	int	k;

	memset(c, 0, sizeof(*c));
	k = 0;
	while (k < IRQ_NVEC)
	{
		c->slot[k].head = IRQ_NONE;
		c->slot[k].gsi = IRQ_NO_GSI;
		k++;
	}
	k = 0;
	while (k < IRQ_ACTIONS_MAX)
	{
		c->act[k].next = IRQ_NONE;
		k++;
	}
}

int	irqc_vec_alloc(t_irq_core *c)
{
	int	k;

	k = 0;
	while (k < IRQ_NVEC)
	{
		if (!c->slot[k].used)
		{
			memset(&c->slot[k], 0, sizeof(c->slot[k]));
			c->slot[k].used = 1;
			c->slot[k].head = IRQ_NONE;
			c->slot[k].gsi = IRQ_NO_GSI;
			return (k);
		}
		k++;
	}
	return (E_NOMEM);
}

void	irqc_vec_release(t_irq_core *c, int idx)
{
	int	k;
	int	next;

	if (idx < 0 || idx >= IRQ_NVEC)
		return ;
	k = c->slot[idx].head;
	while (k != IRQ_NONE)
	{
		next = c->act[k].next;
		memset(&c->act[k], 0, sizeof(c->act[k]));
		c->act[k].next = IRQ_NONE;
		k = next;
	}
	memset(&c->slot[idx], 0, sizeof(c->slot[idx]));
	c->slot[idx].head = IRQ_NONE;
	c->slot[idx].gsi = IRQ_NO_GSI;
}

int	irqc_find_gsi(const t_irq_core *c, uint32_t gsi)
{
	int	k;

	k = 0;
	while (gsi != IRQ_NO_GSI && k < IRQ_NVEC)
	{
		if (c->slot[k].used && !c->slot[k].foreign && c->slot[k].gsi == gsi)
			return (k);
		k++;
	}
	return (IRQ_NONE);
}

uint32_t	irqc_snapshot(const t_irq_core *c, int idx, t_irq_action *out,
	uint32_t max)
{
	uint32_t	n;
	int			k;

	n = 0;
	if (idx < 0 || idx >= IRQ_NVEC)
		return (0);
	k = c->slot[idx].head;
	while (k != IRQ_NONE && n < max)
	{
		out[n] = c->act[k];
		n++;
		k = c->act[k].next;
	}
	return (n);
}
