#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static void	lectures_8_16_32_bits_petit_boutiste(void)
{
	t_pciloc	l;
	uint8_t		*bus;

	bus = fake_ecam_open();
	l = (t_pciloc){0, 2, 3, 1};
	memcpy(bus + (3 << 15) + (1 << 12) + 0x40, "\x78\x56\x34\x12", 4);
	h_eq_u64("32 bits", pcicfg_read(&l, 0x40, 4), 0x12345678);
	h_eq_u64("16 bits bas", pcicfg_read(&l, 0x40, 2), 0x5678);
	h_eq_u64("16 bits haut", pcicfg_read(&l, 0x42, 2), 0x1234);
	h_eq_u64("8 bits (octet 3)", pcicfg_read(&l, 0x43, 1), 0x12);
}

static void	ecritures_n_atteignent_que_leurs_octets(void)
{
	t_pciloc	l;
	uint8_t		*bus;
	uint8_t		*cfg;

	bus = fake_ecam_open();
	l = (t_pciloc){0, 2, 3, 1};
	cfg = bus + (3 << 15) + (1 << 12);
	memset(cfg + 0x40, 0xee, 8);
	pcicfg_write(&l, 0x40, 1, 0x11);
	pcicfg_write(&l, 0x42, 2, 0x4433);
	pcicfg_write(&l, 0x44, 4, 0x88776655);
	h_eq_hex("octets ecrits", cfg + 0x40, 8, "11ee334455667788");
}

static void	hors_fenetre_tout_a_un_et_ecriture_refusee(void)
{
	t_pciloc	l;

	fake_ecam_open();
	l = (t_pciloc){0, 8, 0, 0};
	h_eq_u64("bus 8 hors de 0..7", pcicfg_read(&l, 0, 4), 0xffffffffu);
	h_eq_i64("ecriture hors fenetre", pcicfg_write(&l, 4, 2, 1), E_INVAL);
	l = (t_pciloc){0, 7, 0, 0};
	h_eq_u64("bus 7 dans la fenetre (absent : tout a un)",
		pcicfg_read(&l, 0, 4), 0xffffffffu);
	h_eq_u64("fenetre mappee a la demande", g_fake.io_map_calls, 2);
}

static void	registres_etendus_jusqu_a_4095(void)
{
	t_pciloc	l;
	uint8_t		*bus;

	bus = fake_ecam_open();
	l = (t_pciloc){0, 2, 0, 0};
	memcpy(bus + 0xffc, "\xde\xad\xbe\xef", 4);
	h_eq_u64("dernier dword", pcicfg_read(&l, 0xffc, 4), 0xefbeadde);
	h_eq_u64("hors espace etendu", pcicfg_read(&l, 0x1000, 1), 0xff);
	h_eq_i64("ecriture hors espace", pcicfg_write(&l, 0x1000, 1, 0), E_INVAL);
}

int	main(void)
{
	h_begin("a09/pci_ecam_io");
	h_run("ecam-io/lectures-8-16-32-bits",
		lectures_8_16_32_bits_petit_boutiste);
	h_run("ecam-io/ecritures-par-largeur",
		ecritures_n_atteignent_que_leurs_octets);
	h_run("ecam-io/bus-hors-fenetre",
		hors_fenetre_tout_a_un_et_ecriture_refusee);
	h_run("ecam-io/registres-etendus-0xffc", registres_etendus_jusqu_a_4095);
	return (h_end());
}
