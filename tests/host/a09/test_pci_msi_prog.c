#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static const t_pciloc	g_loc = {0, 0, 2, 0};
static const t_msimsg	g_msg = {0xfee03000u, 0, 0x41};

static void	capacite_64_bits_avec_masquage(void)
{
	t_fabdev	*d;

	fab_reset();
	d = fab_msi_dev(2, 0x0100 | 0x0080 | 0x0070);
	h_eq_i64("activation", pcimsi_enable(&g_loc, 0x50, &g_msg), 0);
	h_eq_u64("adresse basse", fab_get(d, 0x54, 4), 0xfee03000u);
	h_eq_u64("adresse haute", fab_get(d, 0x58, 4), 0);
	h_eq_u64("donnee", fab_get(d, 0x5c, 2), 0x41);
	h_eq_u64("masque du vecteur 0 leve", fab_get(d, 0x60, 4), 0);
	h_eq_u64("MME effacee, enable pose, capacites conservees",
		fab_get(d, 0x52, 2), 0x0100 | 0x0080 | 0x0001);
	h_eq_u64("premiere ecriture : controle sans enable", g_fab.log[0].off,
		0x52);
	h_true(!(g_fab.log[0].val & 1), "enable coupe avant de programmer");
	h_true(g_fab.log[g_fab.nlog - 1].off == 0x52
		&& (g_fab.log[g_fab.nlog - 1].val & 1), "enable ecrit en dernier");
}

static void	capacite_32_bits_sans_masquage(void)
{
	t_fabdev	*d;

	fab_reset();
	d = fab_msi_dev(2, 0x0000);
	h_eq_i64("activation", pcimsi_enable(&g_loc, 0x50, &g_msg), 0);
	h_eq_u64("adresse", fab_get(d, 0x54, 4), 0xfee03000u);
	h_eq_u64("donnee a +8", fab_get(d, 0x58, 2), 0x41);
	h_eq_u64("octets suivants de la donnee non touches",
		fab_get(d, 0x5a, 2), 0);
	h_eq_u64("enable", fab_get(d, 0x52, 2), 1);
	h_eq_u64("donnee ecrite sur 16 bits (pas d'adresse haute)",
		g_fab.log[2].width, 2);
}

static void	activation_non_retenue_et_desactivation(void)
{
	t_fabdev	*d;

	fab_reset();
	d = fab_msi_dev(2, 0x0080);
	d->msi_ro = 1;
	h_eq_i64("enable ignore par le materiel", pcimsi_enable(&g_loc, 0x50,
			&g_msg), E_IO);
	d->msi_ro = 0;
	pcimsi_enable(&g_loc, 0x50, &g_msg);
	pcimsi_disable(&g_loc, 0x50);
	h_eq_u64("enable efface, autres bits conserves", fab_get(d, 0x52, 2),
		0x0080);
}

int	main(void)
{
	h_begin("a09/pci_msi_prog");
	h_run("msi-prog/64-bits-masquage-par-vecteur",
		capacite_64_bits_avec_masquage);
	h_run("msi-prog/32-bits", capacite_32_bits_sans_masquage);
	h_run("msi-prog/enable-ignore-et-desactivation",
		activation_non_retenue_et_desactivation);
	return (h_end());
}
