#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/libk.h"
#include "cpu_int.h"

static t_idttab	g_idt_tab;

static uint64_t	tab_lock(void)
{
	uint64_t	flags;

	flags = irq_save();
	while (__atomic_test_and_set(&g_idt_tab.lock, __ATOMIC_ACQUIRE))
		cpu_relax();
	return (flags);
}

static void	tab_unlock(uint64_t flags)
{
	__atomic_clear(&g_idt_tab.lock, __ATOMIC_RELEASE);
	irq_restore(flags);
}

int	idt_set_handler(uint8_t vec, t_trapfn fn, void *ctx)
{
	uint64_t	flags;
	int			rc;

	if (!fn || (vec < EXC_COUNT && vec != VEC_PAGE_FAULT))
		return (E_INVAL);
	flags = tab_lock();
	rc = E_BUSY;
	if (!g_idt_tab.slots[vec].fn)
	{
		g_idt_tab.slots[vec].ctx = ctx;
		__atomic_store_n(&g_idt_tab.slots[vec].fn, fn, __ATOMIC_RELEASE);
		rc = E_OK;
	}
	tab_unlock(flags);
	return (rc);
}

void	idt_clear_handler(uint8_t vec)
{
	uint64_t	flags;

	flags = tab_lock();
	__atomic_store_n(&g_idt_tab.slots[vec].fn, NULL, __ATOMIC_RELEASE);
	g_idt_tab.slots[vec].ctx = NULL;
	tab_unlock(flags);
}

int	idt_handler_get(uint8_t vec, t_trapslot *out)
{
	out->fn = __atomic_load_n(&g_idt_tab.slots[vec].fn, __ATOMIC_ACQUIRE);
	out->ctx = g_idt_tab.slots[vec].ctx;
	return (out->fn != NULL);
}
