#include <string.h>
#include "a09_test.h"
#include "harness.h"
#include "velum/crypto.h"

static void	ct_equal_partitions(void)
{
	uint8_t	a[64];
	uint8_t	b[64];
	size_t	i;

	fill_pattern(a, sizeof(a), 2104);
	memcpy(b, a, sizeof(b));
	h_eq_i64("buffers egaux", ct_equal(a, b, sizeof(a)), 1);
	h_eq_i64("longueur nulle", ct_equal(a, b, 0), 1);
	h_eq_i64("longueur nulle, pointeurs nuls", ct_equal(NULL, NULL, 0), 1);
	h_eq_i64("pointeur nul, longueur > 0", ct_equal(NULL, b, 4), 0);
	i = 0;
	while (i < sizeof(a))
	{
		b[i] ^= 0x80;
		h_eq_i64("difference sur un octet", ct_equal(a, b, sizeof(a)), 0);
		b[i] ^= 0x80;
		i++;
	}
	h_eq_i64("prefixe egal, suite differente", ct_equal(a, b, 10), 1);
}

static void	ct_equal_valeur_de_retour_binaire(void)
{
	uint8_t	a[8];
	uint8_t	b[8];

	memset(a, 0x00, sizeof(a));
	memset(b, 0xff, sizeof(b));
	h_eq_i64("tous les bits differents", ct_equal(a, b, sizeof(a)), 0);
	b[0] = 0x01;
	memset(b + 1, 0x00, sizeof(b) - 1);
	h_eq_i64("un seul bit different", ct_equal(a, b, sizeof(a)), 0);
}

static void	secure_zero_limites(void)
{
	uint8_t	buf[16];

	memset(buf, 0xaa, sizeof(buf));
	secure_zero(buf + 4, 8);
	h_eq_u64("octet avant la zone intact", buf[3], 0xaa);
	h_eq_u64("premier octet efface", buf[4], 0);
	h_eq_u64("dernier octet efface", buf[11], 0);
	h_eq_u64("octet apres la zone intact", buf[12], 0xaa);
	secure_zero(buf, 0);
	h_eq_u64("taille nulle sans effet", buf[0], 0xaa);
	secure_zero(NULL, 8);
	secure_zero(buf, sizeof(buf));
	h_eq_u64("tout efface", buf[15], 0);
}

int	main(void)
{
	h_begin("a09/ct");
	h_run("ct/equal-partitions-et-limites", ct_equal_partitions);
	h_run("ct/equal-retour-0-ou-1", ct_equal_valeur_de_retour_binaire);
	h_run("ct/secure-zero-limites", secure_zero_limites);
	return (h_end());
}
