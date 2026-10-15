#include <string.h>
#include "a09_test.h"
#include "harness.h"
#include "velum/crypto.h"

static void	check_line(const t_vecline *l)
{
	uint8_t		key[256];
	uint8_t		msg[1024];
	uint8_t		mac[SHA256_LEN];
	t_pbkdf2	k;
	size_t		msg_len;

	h_true(l->n == 4, "ligne de vecteur a quatre champs");
	if (l->n != 4)
		return ;
	memset(&k, 0, sizeof(k));
	k.pw = key;
	k.pwlen = hex_decode(l->f[1], key, sizeof(key));
	msg_len = hex_decode(l->f[2], msg, sizeof(msg));
	hmac_sha256(&k, msg, msg_len, mac);
	h_eq_hex(l->f[0], mac, strlen(l->f[3]) / 2, l->f[3]);
}

static void	hmac_vecteurs_rfc4231_et_longueurs_de_cle(void)
{
	int	lines;

	lines = vec_foreach(VEC_DIR "hmac_sha256.txt", check_line);
	h_true(lines >= 17, "fichier de vecteurs hmac lu en entier");
}

static void	hmac_cle_absente_equivaut_a_cle_vide(void)
{
	t_pbkdf2	k;
	uint8_t		a[SHA256_LEN];
	uint8_t		b[SHA256_LEN];

	memset(&k, 0, sizeof(k));
	hmac_sha256(&k, "message", 7, a);
	hmac_sha256(NULL, "message", 7, b);
	h_true(!memcmp(a, b, SHA256_LEN), "pw nul et pwlen 0 egal a k nul");
	k.pwlen = 5;
	hmac_sha256(&k, "message", 7, b);
	h_true(!memcmp(a, b, SHA256_LEN), "pw nul avec pwlen incoherent ignore");
}

int	main(void)
{
	h_begin("a09/hmac_rfc4231");
	h_run("hmac/rfc4231-cas-1-a-7-et-cles-0-a-200-octets",
		hmac_vecteurs_rfc4231_et_longueurs_de_cle);
	h_run("hmac/cle-absente", hmac_cle_absente_equivaut_a_cle_vide);
	return (h_end());
}
