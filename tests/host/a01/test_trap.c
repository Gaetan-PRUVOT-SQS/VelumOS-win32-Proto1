#include <string.h>
#include "harness.h"
#include "test_a01.h"

static const t_trapcase	g_cases[] = {
{"R1 imbriquee noyau", {13, false, true, false, false}, TRAP_NESTED},
{"R1 MC/DC E faux", {32, false, true, false, false}, TRAP_UNHANDLED},
{"R1 MC/DC U vrai", {13, true, true, false, false}, TRAP_USER_PANIC},
{"R1 MC/DC N faux", {13, false, false, false, false}, TRAP_KERNEL_PANIC},
{"R1 prime sur gestionnaire", {14, false, true, true, false}, TRAP_NESTED},
{"R2 gestionnaire #PF noyau", {14, false, false, true, false},
	TRAP_HANDLER},
{"R2 gestionnaire user ignore N", {14, true, true, true, true},
	TRAP_HANDLER},
{"R2 gestionnaire irq", {0x40, false, true, true, false}, TRAP_HANDLER},
{"R3 irq sans gestionnaire", {0xff, false, false, false, false},
	TRAP_UNHANDLED},
{"R3 limite vecteur 32", {32, true, false, false, true}, TRAP_UNHANDLED},
{"R4 crochet user", {14, true, false, false, true}, TRAP_USER_HOOK},
{"R4 limite vecteur 31", {31, true, false, false, true}, TRAP_USER_HOOK},
{"R4 int3 user", {3, true, false, false, true}, TRAP_USER_HOOK},
{"R5 MC/DC K faux", {14, true, false, false, false}, TRAP_USER_PANIC},
{"R5 MC/DC U faux", {13, false, false, false, true}, TRAP_KERNEL_PANIC},
{"R6 int3 noyau", {3, false, false, false, false}, TRAP_BREAKPOINT},
{"R6 int3 imbrique", {3, false, true, false, false}, TRAP_NESTED},
{"R7 vecteur 4 noyau", {4, false, false, false, false}, TRAP_KERNEL_PANIC},
{"R7 vecteur 0 noyau", {0, false, false, false, true}, TRAP_KERNEL_PANIC},
{NULL, {0, false, false, false, false}, TRAP_HANDLER}
};

static void	table_decision(void)
{
	int	i;

	i = 0;
	while (g_cases[i].name)
	{
		h_eq_i64(g_cases[i].name, trap_classify(&g_cases[i].in),
			g_cases[i].want);
		i++;
	}
}

static void	ist_et_dpl(void)
{
	uint32_t	v;
	int			others;

	h_eq_u64("#DF ist1", idt_vec_ist(8), IST_DOUBLE_FAULT);
	h_eq_u64("NMI ist2", idt_vec_ist(2), IST_NMI);
	h_eq_u64("#MC ist3", idt_vec_ist(18), IST_MCE);
	h_eq_u64("int3 dpl3", idt_vec_dpl(3), 3);
	others = 0;
	v = 0;
	while (v < IDT_VECTORS)
	{
		if (v != 2 && v != 8 && v != 18 && idt_vec_ist(v))
			others++;
		if (v != 3 && idt_vec_dpl(v))
			others++;
		v++;
	}
	h_eq_i64("aucun autre vecteur ist ou dpl3", others, 0);
}

static void	noms_et_options(void)
{
	h_eq_str("nom #DE", trap_name(0), "#DE");
	h_eq_str("nom #PF", trap_name(14), "#PF");
	h_eq_str("nom 31", trap_name(31), "#31");
	h_eq_str("nom irq", trap_name(32), "IRQ");
	h_eq_str("description #DF", trap_desc(8), "double faute");
	h_eq_str("description irq", trap_desc(200), "interruption");
	h_eq_i64("fault div0", fault_parse("div0"), FAULT_DIV0);
	h_eq_i64("fault nmi", fault_parse("nmi"), FAULT_NMI);
	h_eq_i64("fault stack", fault_parse("stack"), FAULT_STACK);
	h_eq_i64("fault inconnu", fault_parse("div"), FAULT_NONE);
	h_eq_i64("fault vide", fault_parse(""), FAULT_NONE);
	h_eq_i64("fault absent", fault_parse(NULL), FAULT_NONE);
	h_eq_str("nom fault pf", fault_name(FAULT_PF), "pf");
	h_eq_str("nom fault hors bornes", fault_name(FAULT_COUNT), "aucune");
	h_eq_str("nom fault negatif", fault_name(-1), "aucune");
}

static void	registres_controle(void)
{
	t_cpufeat	f;
	uint64_t	cr4;

	memset(&f, 0, sizeof(f));
	h_eq_u64("cr0 wp ne mp, em ts effaces", cr0_wanted(CR0_EM | CR0_TS | 1),
		1 | CR0_WP | CR0_NE | CR0_MP);
	cr4 = cr4_wanted(0, &f);
	h_eq_u64("cr4 sans capacite", cr4, CR4_OSFXSR | CR4_OSXMMEXCPT);
	f.xsave = true;
	f.pge = true;
	f.smep = true;
	f.smap = true;
	f.umip = true;
	f.fsgsbase = true;
	cr4 = cr4_wanted(0x20, &f);
	h_eq_u64("cr4 toutes capacites, bits gardes", cr4, 0x20 | CR4_OSFXSR
		| CR4_OSXMMEXCPT | CR4_OSXSAVE | CR4_PGE | CR4_SMEP | CR4_SMAP
		| CR4_UMIP | CR4_FSGSBASE);
	f.smap = false;
	h_eq_u64("smap seulement si annonce", cr4_wanted(0, &f) & CR4_SMAP, 0);
}

int	main(void)
{
	h_begin("a01/trap");
	h_run("table de decision", table_decision);
	h_run("ist et dpl", ist_et_dpl);
	h_run("noms et option fault", noms_et_options);
	h_run("cr0 et cr4", registres_controle);
	return (h_end());
}
