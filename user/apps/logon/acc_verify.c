#include "velum/err.h"
#include "accounts.h"

static int	derive_and_compare(const t_account *a, const char *pw, size_t n)
{
	uint8_t		derived[SHA256_LEN];
	t_pbkdf2	p;
	int			rc;
	int			same;

	p.pw = (const uint8_t *)pw;
	p.pwlen = n;
	p.salt = a->salt;
	p.saltlen = a->salt_len;
	p.iters = a->iterations;
	rc = pbkdf2_sha256(&p, derived, SHA256_LEN);
	same = 0;
	if (rc == 0)
		same = ct_equal(derived, a->hash, SHA256_LEN) != 0;
	secure_zero(derived, sizeof(derived));
	if (rc < 0)
		return (E_INVAL);
	return (same);
}

int	acc_verify(const t_account *a, const char *pw, size_t pwlen)
{
	if (!a || pwlen > ACC_PW_MAX || (pwlen && !pw))
		return (0);
	if (!pw)
		pw = "";
	return (derive_and_compare(a, pw, pwlen));
}

int	acc_needs_password(const t_account *a)
{
	return (acc_verify(a, "", 0) != 1);
}
