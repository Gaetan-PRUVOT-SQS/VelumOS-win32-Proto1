#include <string.h>
#include "harness.h"
#include "velum/err.h"
#include "apkglue.h"
#include "fake.h"

static void	fs_lecture(void)
{
	char	buf[64];
	t_text	out;

	fv_reset();
	out = (t_text){buf, sizeof(buf)};
	h_eq_i64("absent", apkglue_fs()->read(NULL, "/d/a", out), E_NOENT);
	fv_put("/d/a", "contenu de dix-neuf");
	h_eq_i64("present", apkglue_fs()->read(NULL, "/d/a", out), 19);
	h_true(memcmp(buf, "contenu de dix-neuf", 19) == 0, "octets lus");
	out.cap = 19;
	h_eq_i64("tampon juste", apkglue_fs()->read(NULL, "/d/a", out), 19);
	out.cap = 18;
	h_eq_i64("tampon court", apkglue_fs()->read(NULL, "/d/a", out),
		E_OVERFLOW);
	out.cap = sizeof(buf);
	fv_put("/d/b.old", "secours");
	h_eq_i64("repli sur .old", apkglue_fs()->read(NULL, "/d/b", out), 7);
}

static void	fs_ecriture(void)
{
	char	buf[64];
	t_text	out;
	t_span	in;

	fv_reset();
	out = (t_text){buf, sizeof(buf)};
	in = (t_span){(const uint8_t *)"vingt-trois octets ici.", 23};
	h_eq_i64("ecrit", apkglue_fs()->write_new(NULL, "/d/n", in), 0);
	h_eq_i64("relu", apkglue_fs()->read(NULL, "/d/n", out), 23);
	h_true(memcmp(buf, in.p, 23) == 0, "octets relus");
	in.len = 5;
	h_eq_i64("reecrit", apkglue_fs()->write_new(NULL, "/d/n", in), 0);
	h_eq_i64("tronque", apkglue_fs()->read(NULL, "/d/n", out), 5);
	g_fv.fail_at = g_fv.calls + 2;
	in.len = 23;
	h_eq_i64("panne", apkglue_fs()->write_new(NULL, "/d/n", in), E_IO);
	g_fv.fail_at = 0;
	h_eq_i64("mkdir", apkglue_fs()->mkdir(NULL, "/d"), 0);
}

static void	fs_renommage(void)
{
	char	buf[64];
	t_text	out;

	fv_reset();
	out = (t_text){buf, sizeof(buf)};
	fv_put("/d/x.tmp", "nouveau");
	h_eq_i64("sans cible", apkglue_fs()->rename(NULL, "/d/x.tmp", "/d/x"), 0);
	h_eq_i64("lu", apkglue_fs()->read(NULL, "/d/x", out), 7);
	fv_put("/d/x.tmp", "plus neuf");
	h_eq_i64("avec cible", apkglue_fs()->rename(NULL, "/d/x.tmp", "/d/x"), 0);
	h_eq_i64("lu", apkglue_fs()->read(NULL, "/d/x", out), 9);
	h_true(memcmp(buf, "plus neuf", 9) == 0, "nouveau contenu");
	h_true(fv_find("/d/x.old") < 0, ".old supprime");
	h_true(fv_find("/d/x.tmp") < 0, "source partie");
	h_eq_i64("source absente", apkglue_fs()->rename(NULL, "/d/q", "/d/x"),
		E_NOENT);
	h_eq_i64("cible gardee", apkglue_fs()->read(NULL, "/d/x", out), 9);
}

static void	fs_suppression(void)
{
	char	buf[64];
	char	big[300];
	t_text	out;

	fv_reset();
	out = (t_text){buf, sizeof(buf)};
	fv_put("/d/x", "a");
	fv_put("/d/x.old", "b");
	h_eq_i64("supprime", apkglue_fs()->unlink(NULL, "/d/x"), 0);
	h_eq_i64("plus rien", apkglue_fs()->read(NULL, "/d/x", out), E_NOENT);
	h_eq_i64("absent", apkglue_fs()->unlink(NULL, "/d/x"), E_NOENT);
	memset(big, 'a', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	h_eq_i64("chemin long", apkglue_fs()->rename(NULL, "/d/x", big), E_RANGE);
	h_eq_i64("lecture, chemin long", apkglue_fs()->read(NULL, big, out),
		E_NOENT);
}

int	main(void)
{
	h_begin("d12 stockage");
	h_run("fs_lecture", fs_lecture);
	h_run("fs_ecriture", fs_ecriture);
	h_run("fs_renommage", fs_renommage);
	h_run("fs_suppression", fs_suppression);
	return (h_end());
}
