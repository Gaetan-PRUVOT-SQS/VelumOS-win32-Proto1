#include "harness.h"
#include "fake.h"
#include "irq_int.h"
#include "velum/err.h"

#define PCI_SH 0x7
#define LEVEL_SH 0x3

static t_irq_core	g_core;

static void	core_vectors(void)
{
	int	k;
	int	ok;

	irqc_init(&g_core);
	k = 0;
	ok = 1;
	while (k < IRQ_NVEC)
	{
		ok &= (irqc_vec_alloc(&g_core) == k);
		k++;
	}
	h_true(ok, "192 vecteurs dans l'ordre");
	h_eq_i64("vecteur 193 refuse", irqc_vec_alloc(&g_core), E_NOMEM);
	irqc_vec_release(&g_core, 5);
	h_eq_i64("vecteur libere repris", irqc_vec_alloc(&g_core), 5);
	irqc_vec_release(&g_core, -1);
	irqc_vec_release(&g_core, IRQ_NVEC);
	h_eq_i64("gsi absente", irqc_find_gsi(&g_core, IRQ_NO_GSI), IRQ_NONE);
	irqc_init(&g_core);
	g_core.slot[0].used = 1;
	g_core.slot[0].foreign = 1;
	g_core.slot[0].gsi = 3;
	h_eq_i64("vecteur etranger ignore", irqc_find_gsi(&g_core, 3), IRQ_NONE);
	h_eq_i64("vecteur etranger jamais donne", irqc_vec_alloc(&g_core), 1);
}

static void	core_lines(void)
{
	irqc_init(&g_core);
	h_eq_i64("ligne neuve", firq_req(&g_core, 10, ffn_a, 0), 0);
	h_eq_i64("double non partagee", firq_req(&g_core, 10, ffn_b, 0), E_BUSY);
	h_eq_i64("partage sur non partagee", firq_req(&g_core, 10, ffn_b,
			IRQF_SHARED), E_BUSY);
	h_eq_i64("ligne partagee neuve", firq_req(&g_core, 20, ffn_a, PCI_SH), 1);
	h_eq_i64("ligne rejointe", firq_req(&g_core, 20, ffn_b, PCI_SH), 1001);
	h_eq_i64("meme rappel", firq_req(&g_core, 20, ffn_a, PCI_SH), E_EXIST);
	h_eq_i64("declenchement different", firq_req(&g_core, 20, ffn_c,
			LEVEL_SH), E_BUSY);
	h_eq_i64("non partage sur partagee", firq_req(&g_core, 20, ffn_c,
			IRQF_LEVEL | IRQF_LOW), E_BUSY);
}

static void	core_detach(void)
{
	t_irq_action	acts[IRQ_SHARE_MAX];

	irqc_init(&g_core);
	firq_req(&g_core, 20, ffn_a, PCI_SH);
	firq_req(&g_core, 20, ffn_b, PCI_SH);
	h_eq_u64("instantane", irqc_snapshot(&g_core, 0, acts, IRQ_SHARE_MAX), 2);
	h_true(acts[0].fn == ffn_b && acts[1].fn == ffn_a, "ordre des rappels");
	h_eq_u64("instantane borne", irqc_snapshot(&g_core, 0, acts, 1), 1);
	h_eq_u64("instantane hors bornes", irqc_snapshot(&g_core, -1, acts, 1), 0);
	h_eq_i64("retrait reste 1", irqc_detach(&g_core, 0, ffn_a), 1);
	h_eq_i64("retrait absent", irqc_detach(&g_core, 0, ffn_a), E_NOENT);
	h_eq_i64("retrait reste 0", irqc_detach(&g_core, 0, ffn_b), 0);
	g_core.slot[0].nact = IRQ_SHARE_MAX;
	h_eq_i64("partage plein", irqc_attach(&g_core, 0, ffn_c, NULL), E_NOMEM);
}

static void	core_actions(void)
{
	int		k;
	int		ok;
	int		used;

	irqc_init(&g_core);
	k = 0;
	ok = 1;
	while (k < IRQ_ACTIONS_MAX)
	{
		ok &= (firq_req(&g_core, (uint32_t)k, ffn_a, 0) == k);
		k++;
	}
	h_true(ok, "128 lignes");
	h_eq_i64("actions epuisees", firq_req(&g_core, 500, ffn_a, 0), E_NOMEM);
	used = 0;
	k = 0;
	while (k < IRQ_NVEC)
		used += g_core.slot[k++].used;
	h_eq_i64("vecteur rendu apres echec", used, IRQ_ACTIONS_MAX);
	irqc_vec_release(&g_core, 7);
	h_eq_i64("action liberee reprise", firq_req(&g_core, 500, ffn_a, 0), 7);
	h_eq_i64("ancienne ligne oubliee", irqc_find_gsi(&g_core, 7), IRQ_NONE);
}

int	main(void)
{
	h_begin("a05/irq_core");
	h_run("vecteurs alloues et rendus", core_vectors);
	h_run("lignes partagees et refus", core_lines);
	h_run("retrait et instantane", core_detach);
	h_run("epuisement des actions", core_actions);
	return (h_end());
}
