#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"
#include "velum/irq.h"

static void	ligne_isa_traduite_et_drapeaux(void)
{
	const t_pcidev	*d;
	t_fabdev		*f;

	d = fab_plain_device(11, 1);
	f = fab_find(0, 0, 2, 0);
	fab_put(f, 4, 2, 0x0400);
	h_eq_i64("pci_irq_setup", pci_irq_setup(d, fake_nop_isr, NULL), 0);
	h_eq_u64("GSI = isa_to_gsi(11)", g_fake.irq_req_gsi, 111);
	h_eq_u64("partage, niveau, actif bas", g_fake.irq_req_flags,
		IRQF_SHARED | IRQF_LEVEL | IRQF_LOW);
	h_eq_u64("INTx reactive (bit 10 efface)", fab_get(f, 4, 2) & 0x400, 0);
	h_eq_u64("mode memorise", pci_state()->priv[0].irq_mode, PCI_IRQ_INTX);
	h_eq_u64("GSI memorise", pci_state()->priv[0].gsi, 111);
	h_eq_i64("deuxieme appel : E_BUSY", pci_irq_setup(d, fake_nop_isr, NULL),
		E_BUSY);
	h_eq_u64("une seule demande", g_fake.irq_reqs, 1);
}

static void	ligne_au_dela_de_15_prise_telle_quelle(void)
{
	const t_pcidev	*d;

	d = fab_plain_device(23, 4);
	h_eq_i64("pci_irq_setup", pci_irq_setup(d, fake_nop_isr, NULL), 0);
	h_eq_u64("GSI direct", g_fake.irq_req_gsi, 23);
	d = fab_plain_device(15, 2);
	pci_irq_setup(d, fake_nop_isr, NULL);
	h_eq_u64("ligne 15 traduite", g_fake.irq_req_gsi, 115);
	d = fab_plain_device(16, 2);
	pci_irq_setup(d, fake_nop_isr, NULL);
	h_eq_u64("ligne 16 directe", g_fake.irq_req_gsi, 16);
}

static void	pas_d_intx_sans_broche_ni_ligne(void)
{
	const t_pcidev	*d;

	d = fab_plain_device(11, 0);
	h_eq_i64("broche 0", pci_irq_setup(d, fake_nop_isr, NULL), E_NODEV);
	d = fab_plain_device(11, 5);
	h_eq_i64("broche 5", pci_irq_setup(d, fake_nop_isr, NULL), E_NODEV);
	d = fab_plain_device(0, 1);
	h_eq_i64("ligne 0", pci_irq_setup(d, fake_nop_isr, NULL), E_NODEV);
	d = fab_plain_device(0xff, 1);
	h_eq_i64("ligne 0xff (inconnue)", pci_irq_setup(d, fake_nop_isr, NULL),
		E_NODEV);
	h_eq_u64("aucune demande aupres du controleur", g_fake.irq_reqs, 0);
}

static void	echec_du_controleur_et_liberation(void)
{
	const t_pcidev	*d;
	t_fabdev		*f;

	d = fab_plain_device(10, 1);
	f = fab_find(0, 0, 2, 0);
	fab_put(f, 4, 2, 0x0400);
	g_fake.irq_req_rc = E_BUSY;
	h_eq_i64("echec propage", pci_irq_setup(d, fake_nop_isr, NULL), E_BUSY);
	h_eq_u64("mode inchange", pci_state()->priv[0].irq_mode, PCI_IRQ_NONE);
	h_eq_u64("INTx toujours coupe", fab_get(f, 4, 2) & 0x400, 0x400);
	g_fake.irq_req_rc = 0;
	h_eq_i64("nouvel essai", pci_irq_setup(d, fake_nop_isr, NULL), 0);
	pci_irq_free(d);
	h_eq_u64("irq_free appele", g_fake.irq_frees, 1);
	h_eq_u64("avec le meme GSI", g_fake.irq_freed_gsi, 110);
	h_eq_u64("INTx coupe apres liberation", fab_get(f, 4, 2) & 0x400, 0x400);
	pci_irq_free(d);
	h_eq_u64("liberation idempotente", g_fake.irq_frees, 1);
}

int	main(void)
{
	h_begin("a09/pci_irq_intx");
	h_run("irq-intx/ligne-isa-traduite-et-drapeaux",
		ligne_isa_traduite_et_drapeaux);
	h_run("irq-intx/lignes-15-16-23", ligne_au_dela_de_15_prise_telle_quelle);
	h_run("irq-intx/broche-0-5-ligne-0-0xff", pas_d_intx_sans_broche_ni_ligne);
	h_run("irq-intx/echec-controleur-et-liberation",
		echec_du_controleur_et_liberation);
	return (h_end());
}
