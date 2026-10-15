#include "cpu_int.h"

static t_idt	g_idt;

void	idt_setup(void)
{
	const uint64_t	*stubs;
	t_dtr			idtr;
	uint32_t		vec;

	stubs = isr_stub_table();
	vec = 0;
	while (vec < IDT_VECTORS)
	{
		g_idt.gates[vec] = idt_gate_encode(stubs[vec], GDT_KCODE,
				idt_vec_ist(vec), idt_vec_dpl(vec));
		vec++;
	}
	idtr.limit = sizeof(g_idt) - 1;
	idtr.base = (uint64_t)g_idt.gates;
	idt_load(&idtr);
}
