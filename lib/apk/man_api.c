#include "apk_int.h"

static int	man_result(t_apkmanifest *out, int r)
{
	int	reason;

	if (r >= 0)
		return (0);
	reason = out->reason;
	if (reason == APKR_OK)
		reason = APKR_MANIFESTE;
	memset(out, 0, sizeof(*out));
	out->reason = reason;
	return (r);
}

int	apk_manifest(const t_apk *a, t_apkmanifest *out)
{
	t_manwalk	w;
	uint8_t		*buf;
	int64_t		n;
	int			r;

	if (out == NULL)
		return (E_INVAL);
	memset(out, 0, sizeof(*out));
	out->reason = APKR_MANIFESTE;
	n = apk_read(a, "AndroidManifest.xml", &buf);
	if (n == E_NOMEM)
		out->reason = APKR_MEMOIRE;
	if (n < 0)
		return ((int)n);
	memset(&w, 0, sizeof(w));
	w.apk = a;
	w.out = out;
	r = axml_open(&w.x, (t_span){buf, (size_t)n});
	if (r == 0)
		r = man_walk(&w);
	if (r == 0)
		r = man_finish(&w);
	free(buf);
	return (man_result(out, r));
}
