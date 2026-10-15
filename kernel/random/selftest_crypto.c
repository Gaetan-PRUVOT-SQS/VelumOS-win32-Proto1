#include "rng_int.h"
#include "velum/klog.h"
#include "velum/libk.h"

static int	digest_is(const uint8_t *d, size_t n, const char *hex)
{
	char	text[SHA256_LEN * 2 + 1];
	size_t	i;

	i = 0;
	while (i < n && i < SHA256_LEN)
	{
		text[i * 2] = "0123456789abcdef"[d[i] >> 4];
		text[i * 2 + 1] = "0123456789abcdef"[d[i] & 15];
		i++;
	}
	text[i * 2] = '\0';
	return (!strcmp(text, hex));
}

static int	sha_hmac_kat(void)
{
	uint8_t		d[SHA256_LEN];
	t_pbkdf2	k;

	sha256("abc", 3, d);
	if (!digest_is(d, SHA256_LEN, "ba7816bf8f01cfea414140de5dae2223"
			"b00361a396177a9cb410ff61f20015ad"))
		return (1);
	memset(&k, 0, sizeof(k));
	k.pw = (const uint8_t *)"Jefe";
	k.pwlen = 4;
	hmac_sha256(&k, "what do ya want for nothing?", 28, d);
	if (!digest_is(d, SHA256_LEN, "5bdcc146bf60754e6a042426089575c7"
			"5a003f089d2739839dec58b964ec3843"))
		return (2);
	return (0);
}

static int	pbkdf2_chacha_kat(void)
{
	uint8_t		d[SHA256_LEN];
	t_pbkdf2	p;

	memset(&p, 0, sizeof(p));
	p.pw = (const uint8_t *)"passwd";
	p.pwlen = 6;
	p.salt = (const uint8_t *)"salt";
	p.saltlen = 4;
	p.iters = 1;
	if (pbkdf2_sha256(&p, d, sizeof(d)) < 0)
		return (3);
	if (!digest_is(d, SHA256_LEN, "55ac046e56e3089fec1691c22544b605"
			"f94185216dde0465e68b9d57c20dacbc"))
		return (4);
	return (0);
}

static int	ct_wipe_checks(void)
{
	uint8_t	a[8];
	uint8_t	b[8];

	memset(a, 0x5a, sizeof(a));
	memset(b, 0x5a, sizeof(b));
	if (ct_equal(a, b, sizeof(a)) != 1)
		return (5);
	b[7] ^= 1;
	if (ct_equal(a, b, sizeof(a)) != 0)
		return (6);
	secure_zero(a, sizeof(a));
	if (a[0] || a[7])
		return (7);
	return (0);
}

int	crypto_selftest(void)
{
	int	rc;

	rc = sha_hmac_kat();
	if (!rc)
		rc = pbkdf2_chacha_kat();
	if (!rc)
		rc = ct_wipe_checks();
	if (rc)
		klog_err("random: vecteur de test crypto %d en echec", rc);
	return (rc);
}
