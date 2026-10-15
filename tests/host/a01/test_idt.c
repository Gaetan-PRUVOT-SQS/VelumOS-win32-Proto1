#include "harness.h"
#include "velum/err.h"
#include "cpu_int.h"

static void	fn_a(t_regs *regs, void *ctx)
{
	(void)regs;
	(void)ctx;
}

static void	fn_b(t_regs *regs, void *ctx)
{
	*(int *)ctx = (int)regs->vec;
}

static void	partitions_vecteurs(void)
{
	h_eq_i64("vecteur 0 refuse", idt_set_handler(0, fn_a, NULL), E_INVAL);
	h_eq_i64("vecteur 13 refuse", idt_set_handler(13, fn_a, NULL), E_INVAL);
	h_eq_i64("vecteur 15 refuse", idt_set_handler(15, fn_a, NULL), E_INVAL);
	h_eq_i64("limite 0x1f refusee", idt_set_handler(0x1f, fn_a, NULL),
		E_INVAL);
	h_eq_i64("limite 0x20 acceptee", idt_set_handler(0x20, fn_a, NULL), 0);
	h_eq_i64("limite 0xff acceptee", idt_set_handler(0xff, fn_a, NULL), 0);
	h_eq_i64("#PF repris par a03", idt_set_handler(14, fn_a, NULL), 0);
	h_eq_i64("fonction nulle", idt_set_handler(0x40, NULL, NULL), E_INVAL);
	idt_clear_handler(0x20);
	idt_clear_handler(0xff);
	idt_clear_handler(14);
}

static void	transitions_entree(void)
{
	t_trapslot	slot;
	t_regs		regs;
	int			seen;

	seen = 0;
	h_true(!idt_handler_get(0x41, &slot), "libre au depart");
	h_eq_i64("libre -> pris", idt_set_handler(0x41, fn_b, &seen), 0);
	h_eq_i64("pris -> pris refuse", idt_set_handler(0x41, fn_a, NULL),
		E_BUSY);
	h_true(idt_handler_get(0x41, &slot), "lecture prise");
	h_true(slot.fn == fn_b && slot.ctx == &seen, "premier gardé");
	regs.vec = 0x41;
	slot.fn(&regs, slot.ctx);
	h_eq_i64("contexte transmis", seen, 0x41);
	idt_clear_handler(0x41);
	h_true(!idt_handler_get(0x41, &slot), "pris -> libre");
	h_true(slot.fn == NULL && slot.ctx == NULL, "slot vide");
	idt_clear_handler(0x41);
	h_eq_i64("libre -> pris de nouveau", idt_set_handler(0x41, fn_a, NULL),
		0);
	idt_clear_handler(0x41);
}

int	main(void)
{
	h_begin("a01/idt");
	h_run("partitions des vecteurs", partitions_vecteurs);
	h_run("transitions d'une entree", transitions_entree);
	return (h_end());
}
