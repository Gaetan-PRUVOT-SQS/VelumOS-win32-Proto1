#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static const t_fabspec	g_dev = {0, 5, 0, 0x1234, 0x1111, 3, 0, 0};
static const t_pciloc	g_loc = {0, 0, 5, 0};

static void	chaine_msi_msix_pcie(void)
{
	t_fabdev	*d;

	fab_reset();
	d = fab_make(&g_dev);
	fab_put(d, 0x34, 1, 0x50);
	fab_cap(d, 0x50, PCI_CAP_MSI, 0x60);
	fab_cap(d, 0x60, PCI_CAP_MSIX, 0x70);
	fab_cap(d, 0x70, PCI_CAP_PCIE, 0);
	h_eq_i64("MSI", pcicap_next(&g_loc, PCI_CAP_MSI, 0), 0x50);
	h_eq_i64("MSI-X", pcicap_next(&g_loc, PCI_CAP_MSIX, 0), 0x60);
	h_eq_i64("PCIe", pcicap_next(&g_loc, PCI_CAP_PCIE, 0), 0x70);
	h_eq_i64("absente", pcicap_next(&g_loc, 0x09, 0), E_NOENT);
}

static void	capacites_de_meme_type_par_continuation(void)
{
	t_fabdev	*d;

	fab_reset();
	d = fab_make(&g_dev);
	fab_put(d, 0x34, 1, 0x40);
	fab_cap(d, 0x40, 0x09, 0x48);
	fab_cap(d, 0x48, 0x09, 0x50);
	fab_cap(d, 0x50, 0x09, 0);
	h_eq_i64("premiere", pcicap_next(&g_loc, 0x09, 0), 0x40);
	h_eq_i64("seconde", pcicap_next(&g_loc, 0x09, 0x40), 0x48);
	h_eq_i64("troisieme", pcicap_next(&g_loc, 0x09, 0x48), 0x50);
	h_eq_i64("pas de quatrieme", pcicap_next(&g_loc, 0x09, 0x50), E_NOENT);
	h_eq_i64("depart sur une adresse hors liste",
		pcicap_next(&g_loc, 0x09, 0x44), E_NOENT);
}

static void	pointeurs_non_alignes_ou_invalides(void)
{
	t_fabdev	*d;

	fab_reset();
	d = fab_make(&g_dev);
	fab_put(d, 0x34, 1, 0x53);
	fab_cap(d, 0x50, PCI_CAP_MSI, 0x63);
	fab_cap(d, 0x60, PCI_CAP_MSIX, 0x3c);
	h_eq_i64("bits 1:0 du pointeur ignores", pcicap_next(&g_loc, 5, 0), 0x50);
	h_eq_i64("suivant non aligne", pcicap_next(&g_loc, 0x11, 0), 0x60);
	h_eq_i64("pointeur < 0x40 arrete la liste",
		pcicap_next(&g_loc, PCI_CAP_PCIE, 0), E_NOENT);
	fab_put(d, 0x34, 1, 0x30);
	h_eq_i64("tete < 0x40", pcicap_next(&g_loc, PCI_CAP_MSI, 0), E_NOENT);
	fab_put(d, 0x34, 1, 0);
	h_eq_i64("tete nulle", pcicap_next(&g_loc, PCI_CAP_MSI, 0), E_NOENT);
}

static void	boucle_et_longueur_maximale(void)
{
	t_fabdev	*d;
	uint32_t	before;
	int			i;

	fab_reset();
	d = fab_make(&g_dev);
	fab_put(d, 0x34, 1, 0x40);
	fab_cap(d, 0x40, 0x09, 0x40);
	before = g_fab.reads;
	h_eq_i64("capacite qui pointe sur elle-meme", pcicap_next(&g_loc, 5, 0),
		E_NOENT);
	h_true(g_fab.reads - before < 120, "lectures bornees malgre la boucle");
	i = 0x40;
	while (i <= 0xfc)
	{
		fab_cap(d, (uint8_t)i, 0x09, (uint8_t)(i + 4));
		i += 4;
	}
	fab_cap(d, 0xfc, 0x05, 0);
	h_eq_i64("48e capacite trouvee", pcicap_next(&g_loc, 5, 0), 0xfc);
}

int	main(void)
{
	h_begin("a09/pci_cap");
	h_run("cap/chaine-msi-msix-pcie", chaine_msi_msix_pcie);
	h_run("cap/continuation-meme-type",
		capacites_de_meme_type_par_continuation);
	h_run("cap/pointeurs-non-alignes-ou-invalides",
		pointeurs_non_alignes_ou_invalides);
	h_run("cap/boucle-et-48-maximum", boucle_et_longueur_maximale);
	return (h_end());
}
