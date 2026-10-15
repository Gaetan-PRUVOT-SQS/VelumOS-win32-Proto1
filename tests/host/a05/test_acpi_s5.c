#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static const uint8_t	g_byte[] = {0x08, '_', 'S', '5', '_', 0x12, 0x08, 0x04,
	0x0a, 0x05, 0x0a, 0x05, 0x00, 0x00};
static const uint8_t	g_root[] = {0x08, '\\', '_', 'S', '5', '_', 0x12, 0x06,
	0x04, 0x00, 0x01, 0x00, 0x00};
static const uint8_t	g_word[] = {0x08, '_', 'S', '5', '_', 0x12, 0x07, 0x02,
	0x0b, 0x07, 0x00, 0x0a, 0x03};
static const uint8_t	g_long[] = {0x08, '_', 'S', '5', '_', 0x12, 0x46, 0x00,
	0x02, 0x0a, 0x05, 0x01};
static const uint8_t	g_qword[] = {0x08, '_', 'S', '5', '_', 0x12, 0x0c, 0x02,
	0x0e, 3, 0, 0, 0, 0, 0, 0, 0, 0x00};
static const uint8_t	g_two[] = {0x08, '_', 'S', '5', '_', 0x12, 0x05, 0x02,
	0x0a, 0x09, 0x00, 0x08, '_', 'S', '5', '_', 0x12, 0x04, 0x02, 0x01, 0x01};
static const uint8_t	g_bad[] = {0x70, '_', 'S', '5', '_', 0x12, 0x06, 0x04,
	0x00, 0x00, 0x00, 0x00, '_', 'S', '5', '_', 0x08, '_', 'S', '5', '_',
	0x12, 0x04, 0x01, 0x0a, 0x05, 0x08, '_', 'S', '5', '_', 0x12, 0x56, 0x00,
	0x02, 0x00, 0x00, 0x08, '_', 'S', '5', '_', 0x12, 0x05, 0x02, 0xff, 0x00,
	0x08, '_', 'S', '5', '_', 0x12, 0x05, 0x02, 0x0a, 0x08, 0x00, 0x08, '_',
	'S', '5', '_', 0x12, 0x10, 0x02, 0x00};

static void	s5_valid(void)
{
	uint16_t	a;
	uint16_t	b;

	h_eq_i64("prefixe octet", acpi_find_s5(g_byte, sizeof(g_byte), &a, &b),
		E_OK);
	h_true(a == 5 && b == 5, "valeurs 5 5");
	h_eq_i64("nom racine", acpi_find_s5(g_root, sizeof(g_root), &a, &b),
		E_OK);
	h_true(a == 0 && b == 1, "zero et one");
	h_eq_i64("prefixe mot", acpi_find_s5(g_word, sizeof(g_word), &a, &b),
		E_OK);
	h_true(a == 7 && b == 3, "valeurs 7 3");
	h_eq_i64("pkglength 2 octets", acpi_find_s5(g_long, sizeof(g_long), &a,
			&b), E_OK);
	h_true(a == 5 && b == 1, "valeurs 5 1");
	h_eq_i64("prefixe quadruple", acpi_find_s5(g_qword, sizeof(g_qword), &a,
			&b), E_OK);
	h_true(a == 3 && b == 0, "valeurs 3 0");
	h_eq_i64("faux candidat puis vrai", acpi_find_s5(g_two, sizeof(g_two), &a,
			&b), E_OK);
	h_true(a == 1 && b == 1, "deuxieme candidat");
}

static void	s5_invalid(void)
{
	uint16_t	a;
	uint16_t	b;
	uint64_t	n;

	a = 42;
	b = 42;
	h_eq_i64("mutations refusees", acpi_find_s5(g_bad, sizeof(g_bad), &a, &b),
		E_NOENT);
	h_true(a == 42 && b == 42, "sorties intactes");
	h_eq_i64("longueur 0", acpi_find_s5(g_byte, 0, &a, &b), E_NOENT);
	h_eq_i64("longueur 3", acpi_find_s5(g_byte + 1, 3, &a, &b), E_NOENT);
	h_eq_i64("motif en tete sans nameop", acpi_find_s5(g_byte + 1,
			sizeof(g_byte) - 1, &a, &b), E_NOENT);
	n = 1;
	while (n < sizeof(g_byte))
	{
		h_true(acpi_find_s5(g_byte, n, &a, &b) == E_NOENT, "paquet tronque");
		n++;
	}
}

int	main(void)
{
	h_begin("a05/acpi_s5");
	h_run("s5 motifs valides", s5_valid);
	h_run("s5 mutations et troncatures", s5_invalid);
	return (h_end());
}
