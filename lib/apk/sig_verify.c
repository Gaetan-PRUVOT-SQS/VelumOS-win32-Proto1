#include "apk_int.h"

static int	sig_check(const t_apk *a, const t_sigblock *b, const t_signer *s)
{
	uint8_t		h[SHA256_LEN];
	t_span		spki;
	t_rsapub	key;

	if (s->digest.len != SHA256_LEN)
		return (E_ACCES);
	sig_digest(a, b, h);
	if (!ct_equal(h, s->digest.p, SHA256_LEN))
		return (E_ACCES);
	if (x509_spki(s->cert, &spki) < 0 || spki.len != s->pubkey.len
		|| memcmp(spki.p, s->pubkey.p, spki.len) != 0)
		return (E_ACCES);
	if (der_spki_rsa(s->pubkey, &key) < 0 || s->sig.len != key.nlen)
		return (E_ACCES);
	sha256(s->signed_data.p, s->signed_data.len, h);
	if (rsa_pkcs1_sha256_verify(&key, s->sig, h) != 0)
		return (E_ACCES);
	return (0);
}

int	apk_verify(const t_apk *a, t_apksig *out)
{
	t_sigblock	b;
	t_signer	s;
	int			r;

	if (out == NULL)
		return (E_INVAL);
	memset(out, 0, sizeof(*out));
	out->reason = APKR_SIGNATURE;
	if (a == NULL || a->reason != APKR_OK || a->file.p == NULL)
		return (E_ACCES);
	s.reason = APKR_SIGNATURE;
	r = sig_locate(a, &b);
	if (r == 0)
		r = sig_signer(b.v2, &s);
	if (r == 0)
		r = sig_check(a, &b, &s);
	out->reason = s.reason;
	if (r < 0)
		return (r);
	sha256(s.cert.p, s.cert.len, out->cert_sha256);
	out->algo = APK_ALGO_RSA_PKCS1_SHA256;
	out->reason = APKR_OK;
	return (0);
}
