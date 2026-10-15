#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"
#include "velum/vmm.h"

static void	offset_ecam_appareil_fonction_registre(void)
{
	t_pciloc	l;

	l = (t_pciloc){0, 9, 0, 0};
	h_eq_u64("00.0 registre 0", pci_ecam_offset(&l, 0), 0);
	l = (t_pciloc){0, 9, 1, 0};
	h_eq_u64("appareil 1", pci_ecam_offset(&l, 0), 0x8000);
	l = (t_pciloc){0, 9, 0, 1};
	h_eq_u64("fonction 1", pci_ecam_offset(&l, 0), 0x1000);
	l = (t_pciloc){0, 9, 31, 7};
	h_eq_u64("31.7 registre 0xffc", pci_ecam_offset(&l, 0xffc), 0xffffc);
	h_eq_u64("31.7 registre 0x100 (etendu)", pci_ecam_offset(&l, 0x100),
		0xf8000 + 0x7000 + 0x100);
}

static void	init_valide_et_refuse_les_fenetres_incorrectes(void)
{
	t_mcfg_entry	w[3];

	fake_reset();
	pci_state_reset();
	w[0] = (t_mcfg_entry){0xb0000000ull, 0, 0, 255, 0};
	w[1] = (t_mcfg_entry){0xb0000800ull, 0, 0, 255, 0};
	w[2] = (t_mcfg_entry){0, 0, 0, 255, 0};
	h_eq_i64("une fenetre valide sur trois", pci_ecam_init(w, 3), 1);
	w[0] = (t_mcfg_entry){0xc0000000ull, 0, 10, 9, 0};
	h_eq_i64("bus_end < bus_start", pci_ecam_init(w, 1), E_NODEV);
	h_eq_i64("aucune fenetre", pci_ecam_init(w, 0), E_NODEV);
	h_eq_i64("pointeur nul", pci_ecam_init(NULL, 4), E_NODEV);
	h_eq_i64("base non alignee sur 1 Mio", pci_ecam_init(&w[1], 1), E_NODEV);
	h_eq_i64("base nulle", pci_ecam_init(&w[2], 1), E_NODEV);
}

static void	fenetres_en_trop_plafonnees(void)
{
	t_mcfg_entry	w[ACPI_MAX_MCFG + 3];
	int				i;

	fake_reset();
	pci_state_reset();
	i = 0;
	while (i < ACPI_MAX_MCFG + 3)
	{
		w[i] = (t_mcfg_entry){0xb0000000ull + ((uint64_t)i << 28),
			(uint16_t)i, 0, 255, 0};
		i++;
	}
	h_eq_i64("plafond ACPI_MAX_MCFG", pci_ecam_init(w, ACPI_MAX_MCFG + 3),
		ACPI_MAX_MCFG);
	h_eq_i64("derniere fenetre retenue",
		pci_ecam_window(ACPI_MAX_MCFG - 1, 5), ACPI_MAX_MCFG - 1);
	h_eq_i64("fenetre au-dela du plafond", pci_ecam_window(ACPI_MAX_MCFG, 5),
		E_NODEV);
}

static void	recherche_de_fenetre_par_segment_et_bus(void)
{
	t_mcfg_entry	w[3];

	fake_reset();
	pci_state_reset();
	w[0] = (t_mcfg_entry){0xb0000000ull, 0, 0, 63, 0};
	w[1] = (t_mcfg_entry){0xc0000000ull, 0, 100, 199, 0};
	w[2] = (t_mcfg_entry){0xd0000000ull, 1, 0, 255, 0};
	pci_ecam_init(w, 3);
	h_eq_i64("seg 0 bus 0", pci_ecam_window(0, 0), 0);
	h_eq_i64("seg 0 bus 63", pci_ecam_window(0, 63), 0);
	h_eq_i64("seg 0 bus 64", pci_ecam_window(0, 64), E_NODEV);
	h_eq_i64("seg 0 bus 99", pci_ecam_window(0, 99), E_NODEV);
	h_eq_i64("seg 0 bus 100", pci_ecam_window(0, 100), 1);
	h_eq_i64("seg 0 bus 199", pci_ecam_window(0, 199), 1);
	h_eq_i64("seg 0 bus 200", pci_ecam_window(0, 200), E_NODEV);
	h_eq_i64("seg 1 bus 0", pci_ecam_window(1, 0), 2);
	h_eq_i64("seg 2 bus 0", pci_ecam_window(2, 0), E_NODEV);
}

int	main(void)
{
	h_begin("a09/pci_ecam");
	h_run("ecam/offset-appareil-fonction-registre",
		offset_ecam_appareil_fonction_registre);
	h_run("ecam/init-fenetres-valides-et-invalides",
		init_valide_et_refuse_les_fenetres_incorrectes);
	h_run("ecam/plafond-de-fenetres", fenetres_en_trop_plafonnees);
	h_run("ecam/fenetre-par-segment-et-bus-frontieres",
		recherche_de_fenetre_par_segment_et_bus);
	return (h_end());
}
