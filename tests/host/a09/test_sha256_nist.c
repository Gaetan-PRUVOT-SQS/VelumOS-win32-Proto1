#include <stdlib.h>
#include <string.h>
#include "a09_test.h"
#include "harness.h"
#include "velum/crypto.h"

static uint8_t	*build_message(const t_vecline *l, size_t *len)
{
	uint8_t	unit[256];
	uint8_t	*msg;
	size_t	unit_len;
	size_t	rep;
	size_t	i;

	unit_len = hex_decode(l->f[2], unit, sizeof(unit));
	rep = (size_t)strtoul(l->f[1], NULL, 10);
	*len = unit_len * rep;
	msg = malloc(*len + 1);
	i = 0;
	while (msg && i < rep)
	{
		memcpy(msg + i * unit_len, unit, unit_len);
		i++;
	}
	return (msg);
}

static void	check_line(const t_vecline *l)
{
	uint8_t	*msg;
	size_t	len;
	uint8_t	out[SHA256_LEN];

	h_true(l->n == 4, "ligne de vecteur a quatre champs");
	if (l->n != 4)
		return ;
	msg = build_message(l, &len);
	h_true(msg != NULL, "allocation du message");
	if (!msg)
		return ;
	sha256(msg, len, out);
	h_eq_hex(l->f[0], out, SHA256_LEN, l->f[3]);
	free(msg);
}

static void	sha256_vecteurs_nist_et_limites_de_bloc(void)
{
	int	lines;

	lines = vec_foreach(VEC_DIR "sha256.txt", check_line);
	h_true(lines >= 25, "fichier de vecteurs sha256 lu en entier");
}

int	main(void)
{
	h_begin("a09/sha256_nist");
	h_run("sha256/vecteurs-nist-et-bornes-de-remplissage",
		sha256_vecteurs_nist_et_limites_de_bloc);
	return (h_end());
}
