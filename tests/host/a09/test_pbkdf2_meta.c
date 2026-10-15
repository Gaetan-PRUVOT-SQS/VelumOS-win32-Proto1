#include <string.h>
#include "a09_test.h"
#include "harness.h"
#include "velum/crypto.h"
#include "velum/err.h"

static void	prefixe_de_cle_plus_longue(void)
{
	t_pbkdf2	p;
	uint8_t		longue[100];
	uint8_t		courte[100];
	size_t		len;
	int			bad;

	memset(&p, 0, sizeof(p));
	p.pw = (const uint8_t *)"secret";
	p.pwlen = 6;
	p.salt = (const uint8_t *)"sel";
	p.saltlen = 3;
	p.iters = 11;
	h_eq_i64("derivation 100 octets", pbkdf2_sha256(&p, longue, 100), 0);
	bad = 0;
	len = 1;
	while (len <= 100)
	{
		bad += pbkdf2_sha256(&p, courte, len) != 0;
		bad += memcmp(courte, longue, len) != 0;
		len++;
	}
	h_eq_i64("cles courtes non prefixes de la longue", bad, 0);
}

static void	un_octet_de_sel_change_tout(void)
{
	t_pbkdf2	p;
	uint8_t		a[32];
	uint8_t		b[32];

	memset(&p, 0, sizeof(p));
	p.pw = (const uint8_t *)"secret";
	p.pwlen = 6;
	p.salt = (const uint8_t *)"sel1";
	p.saltlen = 4;
	p.iters = 3;
	pbkdf2_sha256(&p, a, sizeof(a));
	p.salt = (const uint8_t *)"sel2";
	pbkdf2_sha256(&p, b, sizeof(b));
	h_true(memcmp(a, b, sizeof(a)) != 0, "sel different, cle differente");
}

int	main(void)
{
	h_begin("a09/pbkdf2_meta");
	h_run("pbkdf2/metamorphique-prefixe-dklen-1-a-100",
		prefixe_de_cle_plus_longue);
	h_run("pbkdf2/metamorphique-sel", un_octet_de_sel_change_tout);
	return (h_end());
}
