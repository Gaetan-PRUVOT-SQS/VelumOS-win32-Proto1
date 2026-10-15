#include <string.h>
#include "a09_test.h"
#include "crypto_int.h"
#include "harness.h"
#include "velum/err.h"

static t_pbkdf2	base_params(uint32_t iters)
{
	t_pbkdf2	p;

	memset(&p, 0, sizeof(p));
	p.pw = (const uint8_t *)"pw";
	p.pwlen = 2;
	p.salt = (const uint8_t *)"sel";
	p.saltlen = 3;
	p.iters = iters;
	return (p);
}

static void	iterations_valeurs_limites(void)
{
	t_pbkdf2	p;

	p = base_params(0);
	h_eq_i64("0 iteration refusee", pbkdf2_validate(&p, 32), E_INVAL);
	p.iters = 1;
	h_eq_i64("1 iteration acceptee", pbkdf2_validate(&p, 32), 0);
	p.iters = 10000000;
	h_eq_i64("10 000 000 acceptee", pbkdf2_validate(&p, 32), 0);
	p.iters = 10000001;
	h_eq_i64("10 000 001 refusee", pbkdf2_validate(&p, 32), E_INVAL);
	p.iters = 0xffffffffu;
	h_eq_i64("UINT32_MAX refusee", pbkdf2_validate(&p, 32), E_INVAL);
}

static void	pointeurs_et_longueurs_incoherents(void)
{
	t_pbkdf2	p;
	uint8_t		out[32];

	h_eq_i64("parametres nuls", pbkdf2_sha256(NULL, out, 32), E_INVAL);
	p = base_params(2);
	h_eq_i64("sortie nulle", pbkdf2_sha256(&p, NULL, 32), E_INVAL);
	h_eq_i64("dkLen nul", pbkdf2_sha256(&p, out, 0), E_INVAL);
	p.pw = NULL;
	h_eq_i64("pw nul avec pwlen > 0", pbkdf2_sha256(&p, out, 32), E_INVAL);
	p.pwlen = 0;
	h_eq_i64("pw nul avec pwlen 0 accepte", pbkdf2_sha256(&p, out, 32), 0);
	p.salt = NULL;
	h_eq_i64("sel nul avec saltlen > 0", pbkdf2_sha256(&p, out, 32), E_INVAL);
	p.saltlen = 0;
	h_eq_i64("sel nul avec saltlen 0 accepte", pbkdf2_sha256(&p, out, 32), 0);
}

static void	dklen_valeurs_limites(void)
{
	t_pbkdf2	p;
	size_t		max_len;

	p = base_params(1);
	max_len = (size_t)0xffffffffu * SHA256_LEN;
	h_eq_i64("dkLen = (2^32-1)*32 accepte", pbkdf2_validate(&p, max_len), 0);
	h_eq_i64("dkLen = (2^32-1)*32+1 refuse",
		pbkdf2_validate(&p, max_len + 1), E_INVAL);
	h_eq_i64("dkLen = SIZE_MAX refuse", pbkdf2_validate(&p, (size_t)-1),
		E_INVAL);
	h_eq_i64("dkLen = 1 accepte", pbkdf2_validate(&p, 1), 0);
}

int	main(void)
{
	h_begin("a09/pbkdf2_args");
	h_run("pbkdf2/iterations-0-1-10M-10M+1-max", iterations_valeurs_limites);
	h_run("pbkdf2/pointeurs-nuls-et-longueurs",
		pointeurs_et_longueurs_incoherents);
	h_run("pbkdf2/dklen-limites-rfc8018", dklen_valeurs_limites);
	return (h_end());
}
