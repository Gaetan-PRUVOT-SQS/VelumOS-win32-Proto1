#include "a09_test.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static void	encodage_adresse_et_donnee(void)
{
	t_msimsg	m;

	h_eq_i64("apic 0 vecteur 0x30", pci_msi_encode(0, 0x30, &m), 0);
	h_eq_u64("adresse basse : 0xfee00000", m.addr_lo, 0xfee00000u);
	h_eq_u64("adresse haute", m.addr_hi, 0);
	h_eq_u64("donnee = vecteur (front, fixe)", m.data, 0x30);
	h_eq_i64("apic 3 vecteur 0x41", pci_msi_encode(3, 0x41, &m), 0);
	h_eq_u64("destination en bits 19:12", m.addr_lo, 0xfee03000u);
	h_eq_u64("donnee 0x41", m.data, 0x41);
	h_eq_i64("apic 255", pci_msi_encode(255, 0xfe, &m), 0);
	h_eq_u64("destination 0xff", m.addr_lo, 0xfeeff000u);
	h_eq_u64("vecteur 0xfe", m.data, 0xfe);
}

static void	bornes_du_vecteur(void)
{
	t_msimsg	m;

	m.addr_lo = 0xdeadbeefu;
	h_eq_i64("vecteur 0x0f", pci_msi_encode(0, 0x0f, &m), E_INVAL);
	h_eq_i64("vecteur 0x10", pci_msi_encode(0, 0x10, &m), 0);
	h_eq_i64("vecteur 0xfe", pci_msi_encode(0, 0xfe, &m), 0);
	m.addr_lo = 0xdeadbeefu;
	h_eq_i64("vecteur 0xff (parasite)", pci_msi_encode(0, 0xff, &m), E_INVAL);
	h_eq_i64("vecteur 0x100", pci_msi_encode(0, 0x100, &m), E_INVAL);
	h_eq_i64("vecteur negatif", pci_msi_encode(0, -1, &m), E_INVAL);
	h_eq_u64("message intact apres refus", m.addr_lo, 0xdeadbeefu);
}

static void	identifiants_apic_au_dela_de_8_bits(void)
{
	t_msimsg	m;

	h_eq_i64("apic 256 : remappage requis", pci_msi_encode(256, 0x40, &m),
		E_RANGE);
	h_eq_i64("apic 0xffffffff", pci_msi_encode(0xffffffffu, 0x40, &m),
		E_RANGE);
	h_eq_i64("apic 255 reste valide", pci_msi_encode(255, 0x40, &m), 0);
}

int	main(void)
{
	h_begin("a09/pci_msi_encode");
	h_run("msi-encode/adresse-et-donnee-intel-sdm-11.11",
		encodage_adresse_et_donnee);
	h_run("msi-encode/vecteurs-0x0f-0x10-0xfe-0xff", bornes_du_vecteur);
	h_run("msi-encode/apic-255-256", identifiants_apic_au_dela_de_8_bits);
	return (h_end());
}
