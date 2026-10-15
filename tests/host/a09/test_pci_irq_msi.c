#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"
#include "velum/irq.h"

static void	configuration_complete_d_un_msi(void)
{
	const t_pcidev	*d;
	t_fabdev		*f;

	d = fab_msi_device(0x0080);
	f = fab_find(0, 0, 2, 0);
	g_fake.apic = 3;
	g_fake.vec_next = 0x41;
	h_eq_i64("pci_irq_setup", pci_irq_setup(d, fake_nop_isr, NULL), 0);
	h_eq_u64("un vecteur alloue", g_fake.vec_allocs, 1);
	h_eq_u64("aucune ligne INTx demandee", g_fake.irq_reqs, 0);
	h_eq_u64("adresse : LAPIC 3", fab_get(f, 0x54, 4), 0xfee03000u);
	h_eq_u64("donnee : vecteur 0x41", fab_get(f, 0x5c, 2), 0x41);
	h_eq_u64("MSI active", fab_get(f, 0x52, 2) & 1, 1);
	h_eq_u64("INTx coupe dans la commande", fab_get(f, 4, 2) & 0x400, 0x400);
	h_eq_u64("mode memorise", pci_state()->priv[0].irq_mode, PCI_IRQ_MSI);
	h_eq_u64("vecteur memorise", (uint64_t)pci_state()->priv[0].vector, 0x41);
}

static void	un_seul_appel_par_appareil_puis_liberation(void)
{
	const t_pcidev	*d;
	t_fabdev		*f;

	d = fab_msi_device(0x0080);
	f = fab_find(0, 0, 2, 0);
	g_fake.vec_next = 0x41;
	pci_irq_setup(d, fake_nop_isr, NULL);
	h_eq_i64("deuxieme appel : E_BUSY", pci_irq_setup(d, fake_nop_isr, NULL),
		E_BUSY);
	h_eq_u64("pas de second vecteur", g_fake.vec_allocs, 1);
	pci_irq_free(d);
	h_eq_u64("MSI desactive", fab_get(f, 0x52, 2) & 1, 0);
	h_eq_u64("vecteur rendu", g_fake.vec_frees, 1);
	h_eq_u64("vecteur rendu : 0x41", (uint64_t)g_fake.vec_last_freed, 0x41);
	h_eq_u64("mode remis a zero", pci_state()->priv[0].irq_mode, PCI_IRQ_NONE);
	h_eq_i64("nouvel appel possible", pci_irq_setup(d, fake_nop_isr, NULL), 0);
	pci_irq_free(d);
	pci_irq_free(d);
	h_eq_u64("liberation idempotente", g_fake.vec_frees, 2);
}

static void	repli_sur_intx_si_plus_de_vecteur(void)
{
	const t_pcidev	*d;
	t_fabdev		*f;

	d = fab_msi_device(0x0080);
	f = fab_find(0, 0, 2, 0);
	fab_irq(f, 10, 1);
	pci_state()->devs[0].irq_line = 10;
	pci_state()->devs[0].irq_pin = 1;
	g_fake.vec_fail = E_NOMEM;
	h_eq_i64("repli INTx", pci_irq_setup(d, fake_nop_isr, NULL), 0);
	h_eq_u64("INTx : GSI de la ligne 10 via les overrides", g_fake.irq_req_gsi,
		110);
	h_eq_u64("MSI jamais active", fab_get(f, 0x52, 2) & 1, 0);
	h_eq_u64("mode INTx", pci_state()->priv[0].irq_mode, PCI_IRQ_INTX);
}

static void	apic_trop_grand_et_activation_refusee(void)
{
	const t_pcidev	*d;
	t_fabdev		*f;

	d = fab_msi_device(0x0080);
	f = fab_find(0, 0, 2, 0);
	g_fake.apic = 300;
	h_eq_i64("apic > 255 sans INTx : E_NODEV",
		pci_irq_setup(d, fake_nop_isr, NULL), E_NODEV);
	h_eq_u64("vecteur rendu", g_fake.vec_frees, 1);
	h_eq_u64("registres MSI non touches", fab_writes_at(0x50, 0x64), 0);
	g_fake.apic = 0;
	f->msi_ro = 1;
	h_eq_i64("enable ignore : E_NODEV", pci_irq_setup(d, fake_nop_isr, NULL),
		E_NODEV);
	h_eq_u64("second vecteur rendu", g_fake.vec_frees, 2);
	h_eq_u64("mode inchange", pci_state()->priv[0].irq_mode, PCI_IRQ_NONE);
}

int	main(void)
{
	h_begin("a09/pci_irq_msi");
	h_run("irq-msi/configuration-complete", configuration_complete_d_un_msi);
	h_run("irq-msi/un-appel-par-appareil-et-liberation",
		un_seul_appel_par_appareil_puis_liberation);
	h_run("irq-msi/repli-intx-vecteur-epuise",
		repli_sur_intx_si_plus_de_vecteur);
	h_run("irq-msi/apic-256-et-enable-refuse",
		apic_trop_grand_et_activation_refusee);
	return (h_end());
}
