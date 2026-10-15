#include "velum/err.h"
#include "velum/libk.h"
#include "apkglue.h"

static int	g_apkglue_reason;

static int	refuse(int err, int reason, int fallback)
{
	g_apkglue_reason = reason;
	if (reason <= APKR_OK || reason >= APKR_COUNT)
		g_apkglue_reason = fallback;
	return (err);
}

static void	fill_info(t_pminfo *out, const t_apkmanifest *m,
		const t_apksig *sig)
{
	memset(out, 0, sizeof(*out));
	apkglue_label(out->package, PM_PKG_MAX, m->package);
	apkglue_label(out->label, PM_LABEL_MAX, m->label);
	apkglue_label(out->activity, PM_ACT_MAX, m->activity_desc);
	memcpy(out->cert, sig->cert_sha256, PM_CERT_LEN);
	out->version_code = m->version_code;
}

int	apkglue_inspect(t_span apk, t_pminfo *out)
{
	t_apkmanifest	man;
	t_apksig		sig;
	t_apk			a;
	int				r;

	g_apkglue_reason = APKR_OK;
	memset(&sig, 0, sizeof(sig));
	memset(&man, 0, sizeof(man));
	r = apk_open(&a, apk);
	if (r < 0)
		return (refuse(r, a.reason, APKR_ARCHIVE));
	r = apk_verify(&a, &sig);
	if (r < 0)
		return (refuse(r, sig.reason, APKR_SIGNATURE));
	r = apk_manifest(&a, &man);
	if (r < 0)
		return (refuse(r, man.reason, APKR_MANIFESTE));
	fill_info(out, &man, &sig);
	return (0);
}

int	apkglue_reason(void)
{
	return (g_apkglue_reason);
}
