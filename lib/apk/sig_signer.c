#include "apk_int.h"

static int	algo_known(uint32_t id)
{
	return ((id >= 0x0101 && id <= 0x0104) || id == 0x0201 || id == 0x0202
		|| id == 0x0301);
}

static int	sig_item(t_span item, uint32_t *algo, t_span *value)
{
	t_span	rest;

	if (item.len < 8)
		return (E_ACCES);
	*algo = apk_rd32(item.p);
	rest = (t_span){item.p + 4, item.len - 4};
	if (apk_lp(&rest, value) < 0 || rest.len != 0 || !algo_known(*algo))
		return (E_ACCES);
	return (0);
}

static int	sig_algos(t_span digests, t_span sigs, t_signer *s)
{
	t_span		d;
	t_span		g;
	uint32_t	ad;
	uint32_t	ag;

	if (digests.len == 0)
		return (E_ACCES);
	while (digests.len > 0 || sigs.len > 0)
	{
		if (apk_lp(&digests, &d) < 0 || apk_lp(&sigs, &g) < 0)
			return (E_ACCES);
		if (sig_item(d, &ad, &d) < 0 || sig_item(g, &ag, &g) < 0 || ad != ag)
			return (E_ACCES);
		if (ad == APK_ALGO_RSA_PKCS1_SHA256 && s->found)
			return (E_ACCES);
		if (ad == APK_ALGO_RSA_PKCS1_SHA256)
		{
			s->digest = d;
			s->sig = g;
			s->found = 1;
		}
	}
	return (0);
}

static int	sig_signed(t_signer *s, t_span sigs)
{
	t_span	sd;
	t_span	digests;
	t_span	certs;
	t_span	item;

	sd = s->signed_data;
	if (apk_lp(&sd, &digests) < 0 || apk_lp(&sd, &certs) < 0
		|| apk_lp(&sd, &item) < 0 || apk_lp(&certs, &s->cert) < 0)
		return (E_ACCES);
	while (certs.len > 0)
		if (apk_lp(&certs, &sd) < 0)
			return (E_ACCES);
	while (item.len > 0)
		if (apk_lp(&item, &sd) < 0 || sd.len < 4)
			return (E_ACCES);
	if (sig_algos(digests, sigs, s) < 0)
		return (E_ACCES);
	if (!s->found)
	{
		s->reason = APKR_ALGO;
		return (E_NOTSUP);
	}
	return (0);
}

int	sig_signer(t_span v2, t_signer *s)
{
	t_span	signers;
	t_span	one;
	t_span	sigs;

	memset(s, 0, sizeof(*s));
	s->reason = APKR_SIGNATURE;
	if (apk_lp(&v2, &signers) < 0 || v2.len != 0
		|| apk_lp(&signers, &one) < 0)
		return (E_ACCES);
	if (signers.len != 0)
	{
		s->reason = APKR_SIGNATAIRES;
		return (E_NOTSUP);
	}
	if (apk_lp(&one, &s->signed_data) < 0 || apk_lp(&one, &sigs) < 0
		|| apk_lp(&one, &s->pubkey) < 0 || one.len != 0)
		return (E_ACCES);
	return (sig_signed(s, sigs));
}
