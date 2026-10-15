#include <stddef.h>
#include <string.h>
#include "harness.h"
#include "cpu_int.h"

static void	regs_disposition(void)
{
	h_eq_u64("taille t_regs", sizeof(t_regs), 176);
	h_eq_u64("r15 en tete", offsetof(t_regs, r15), 0);
	h_eq_u64("rax", offsetof(t_regs, rax), 112);
	h_eq_u64("vec apres les 15 registres", offsetof(t_regs, vec), 120);
	h_eq_u64("err", offsetof(t_regs, err), 128);
	h_eq_u64("rip trame cpu", offsetof(t_regs, rip), 136);
	h_eq_u64("cs = vec + 24 (isr.S)", offsetof(t_regs, cs),
		offsetof(t_regs, vec) + 24);
	h_eq_u64("rsp", offsetof(t_regs, rsp), 160);
	h_eq_u64("ss en fin", offsetof(t_regs, ss), 168);
	h_eq_u64("taille multiple de 16", sizeof(t_regs) % 16, 0);
}

static void	cpu_offsets(void)
{
	h_eq_u64("CPU_OFF_SELF", offsetof(t_cpu, self), CPU_OFF_SELF);
	h_eq_u64("CPU_OFF_KSTACK", offsetof(t_cpu, kstack_top), CPU_OFF_KSTACK);
	h_eq_u64("CPU_OFF_USERRSP", offsetof(t_cpu, user_rsp), CPU_OFF_USERRSP);
	h_eq_u64("CPU_OFF_CURRENT", offsetof(t_cpu, current), CPU_OFF_CURRENT);
}

static void	tss_disposition(void)
{
	h_eq_u64("taille tss SDM", sizeof(t_tss), 104);
	h_eq_u64("rsp0 a 4", offsetof(t_tss, rsp), 4);
	h_eq_u64("ist1 a 36", offsetof(t_tss, ist), 36);
	h_eq_u64("iomap a 102", offsetof(t_tss, iomap_base), 102);
	h_eq_u64("taille porte idt", sizeof(t_idt_gate), 16);
	h_eq_u64("taille gdtr", sizeof(t_dtr), 10);
	h_eq_u64("taille gdt", sizeof(t_gdt), 56);
}

static void	tss_remplissage(void)
{
	t_tss	t;

	memset(&t, 0xa5, sizeof(t));
	tss_fill(&t, 0x1000, 0x2000, 0x3000);
	h_eq_u64("ist1 double faute", t.ist[IST_DOUBLE_FAULT - 1], 0x1000);
	h_eq_u64("ist2 nmi", t.ist[IST_NMI - 1], 0x2000);
	h_eq_u64("ist3 mce", t.ist[IST_MCE - 1], 0x3000);
	h_eq_u64("ist4 vide", t.ist[3], 0);
	h_eq_u64("rsp0 nul", t.rsp[0], 0);
	h_eq_u64("bitmap absente : offset > limite", t.iomap_base, 104);
	h_true(t.iomap_base > sizeof(t_tss) - 1, "offset au-dela de la limite");
}

int	main(void)
{
	h_begin("a01/layout");
	h_run("t_regs", regs_disposition);
	h_run("t_cpu", cpu_offsets);
	h_run("tss et tables", tss_disposition);
	h_run("tss_fill", tss_remplissage);
	return (h_end());
}
