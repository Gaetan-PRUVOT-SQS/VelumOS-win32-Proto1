#include <stdlib.h>
#include <string.h>
#include "a09_test.h"
#include "harness.h"
#include "velum/crypto.h"

#define GUARD 8

static void	derive_and_compare(const t_vecline *l, const t_pbkdf2 *p,
		size_t dk_len)
{
	uint8_t	*buf;
	uint8_t	guard[GUARD];
	int		rc;

	buf = malloc(dk_len + GUARD);
	h_true(buf != NULL, "allocation de la cle derivee");
	if (!buf)
		return ;
	memset(guard, 0xa5, GUARD);
	memcpy(buf + dk_len, guard, GUARD);
	rc = pbkdf2_sha256(p, buf, dk_len);
	h_eq_i64("code de retour", rc, 0);
	h_eq_hex(l->f[0], buf, dk_len, l->f[4]);
	h_true(!memcmp(buf + dk_len, guard, GUARD),
		"aucun octet ecrit apres dkLen");
	free(buf);
}

static void	check_line(const t_vecline *l)
{
	uint8_t		pw[256];
	uint8_t		salt[256];
	t_pbkdf2	p;

	h_true(l->n == 5, "ligne de vecteur a cinq champs");
	if (l->n != 5)
		return ;
	memset(&p, 0, sizeof(p));
	p.pw = pw;
	p.pwlen = hex_decode(l->f[2], pw, sizeof(pw));
	p.salt = salt;
	p.saltlen = hex_decode(l->f[3], salt, sizeof(salt));
	p.iters = (uint32_t)strtoul(l->f[1], NULL, 10);
	derive_and_compare(l, &p, strlen(l->f[4]) / 2);
}

static void	pbkdf2_vecteurs_rfc7914_et_oracle(void)
{
	int	lines;

	lines = vec_foreach(VEC_DIR "pbkdf2_sha256.txt", check_line);
	h_true(lines >= 17, "fichier de vecteurs pbkdf2 lu en entier");
}

int	main(void)
{
	h_begin("a09/pbkdf2_kat");
	h_run("pbkdf2/rfc7914-11-1-4096-iterations-sorties-1-a-100",
		pbkdf2_vecteurs_rfc7914_et_oracle);
	return (h_end());
}
