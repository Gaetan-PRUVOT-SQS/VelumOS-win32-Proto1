#include <string.h>
#include "a09_test.h"
#include "crypto_int.h"
#include "harness.h"
#include "velum/util.h"

static int	incremental_matches(const uint8_t *key, const uint8_t *msg,
		size_t chunk)
{
	t_hmac		h;
	t_pbkdf2	k;
	uint8_t		whole[SHA256_LEN];
	uint8_t		parts[SHA256_LEN];
	size_t		off;

	memset(&k, 0, sizeof(k));
	k.pw = key;
	k.pwlen = 70;
	hmac_sha256(&k, msg, 400, whole);
	hmac_prepare(&h, key, 70);
	off = 0;
	while (off < 400)
	{
		hmac_update(&h, msg + off, min_u64(chunk, 400 - off));
		off += chunk;
	}
	hmac_final(&h, parts);
	return (!memcmp(whole, parts, SHA256_LEN));
}

static void	hmac_incremental_equivalent_a_un_appel(void)
{
	uint8_t	key[70];
	uint8_t	msg[400];
	size_t	chunk;
	int		bad;

	fill_pattern(key, sizeof(key), 4231);
	fill_pattern(msg, sizeof(msg), 8018);
	bad = 0;
	chunk = 1;
	while (chunk <= 200)
	{
		bad += !incremental_matches(key, msg, chunk);
		chunk++;
	}
	h_eq_i64("decoupages hmac divergents (graines 4231 et 8018)", bad, 0);
}

static void	hmac_modele_reutilisable(void)
{
	t_hmac		tmpl;
	t_hmac		a;
	t_hmac		b;
	uint8_t		out_a[SHA256_LEN];
	uint8_t		out_b[SHA256_LEN];

	hmac_prepare(&tmpl, (const uint8_t *)"cle", 3);
	a = tmpl;
	b = tmpl;
	hmac_update(&a, "un", 2);
	hmac_update(&b, "un", 2);
	hmac_final(&a, out_a);
	hmac_final(&b, out_b);
	h_true(!memcmp(out_a, out_b, SHA256_LEN),
		"deux copies du modele concordent");
}

int	main(void)
{
	h_begin("a09/hmac_split");
	h_run("hmac/incremental-1-a-200-octets-cle-70-octets",
		hmac_incremental_equivalent_a_un_appel);
	h_run("hmac/modele-copiable", hmac_modele_reutilisable);
	return (h_end());
}
