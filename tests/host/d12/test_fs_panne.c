#include <string.h>
#include "harness.h"
#include "velum/err.h"
#include "apkglue.h"
#include "fake.h"

static void	prepare(int fail_at, int cut, int stale)
{
	fv_reset();
	fv_put("/d/reg", "ancien");
	fv_put("/d/reg.tmp", "nouveau");
	if (stale)
		fv_put("/d/reg.old", "perime");
	g_fv.fail_at = fail_at;
	g_fv.cut = cut;
}

static int	relisible(int r)
{
	char	buf[32];
	t_text	out;
	int64_t	n;

	g_fv.fail_at = 0;
	g_fv.cut = 0;
	out = (t_text){buf, sizeof(buf)};
	n = apkglue_fs()->read(NULL, "/d/reg", out);
	if (n == 7 && memcmp(buf, "nouveau", 7) == 0)
		return (1);
	if (r == 0)
		return (0);
	return (n == 6 && memcmp(buf, "ancien", 6) == 0);
}

static void	panne_unique(void)
{
	int	k;
	int	r;

	k = 1;
	while (k <= 8)
	{
		prepare(k, 0, 0);
		r = apkglue_fs()->rename(NULL, "/d/reg.tmp", "/d/reg");
		h_true(relisible(r), "panne unique : fichier relisible");
		prepare(k, 0, 1);
		r = apkglue_fs()->rename(NULL, "/d/reg.tmp", "/d/reg");
		h_true(relisible(r), "panne unique, .old perime : relisible");
		k++;
	}
	prepare(0, 0, 1);
	h_eq_i64("sans panne", apkglue_fs()->rename(NULL, "/d/reg.tmp", "/d/reg"),
		0);
	h_true(relisible(0), "sans panne : nouveau");
}

static void	panne_coupure(void)
{
	int	k;
	int	r;

	k = 1;
	while (k <= 8)
	{
		prepare(k, 1, 0);
		r = apkglue_fs()->rename(NULL, "/d/reg.tmp", "/d/reg");
		h_true(r < 0 || k > 3, "coupure : erreur rendue");
		h_true(relisible(r), "coupure : fichier relisible");
		prepare(k, 1, 1);
		r = apkglue_fs()->rename(NULL, "/d/reg.tmp", "/d/reg");
		h_true(relisible(r), "coupure, .old perime : relisible");
		r = apkglue_fs()->rename(NULL, "/d/reg.tmp", "/d/reg");
		h_true(relisible(r), "reprise apres coupure : relisible");
		k++;
	}
}

int	main(void)
{
	h_begin("d12 pannes du renommage");
	h_run("panne_unique", panne_unique);
	h_run("panne_coupure", panne_coupure);
	return (h_end());
}
