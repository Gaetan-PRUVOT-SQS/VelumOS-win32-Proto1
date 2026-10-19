#include "fake.h"
#include "apkrun_trust.h"

static t_pm	g_pm;

static void	put_system_app(const char *pkg, uint8_t signer)
{
	static t_pmentry	e;
	static char			text[FAKE_DATA];
	t_text				out;
	int					n;

	memset(&e, 0, sizeof(e));
	strlcpy(e.info.package, pkg, PM_PKG_MAX);
	strlcpy(e.info.label, "Systeme", PM_LABEL_MAX);
	strlcpy(e.info.activity, "Lx/y/Main;", PM_ACT_MAX);
	memset(e.info.cert, signer, PM_CERT_LEN);
	e.info.version_code = 1;
	out.p = text;
	out.cap = sizeof(text);
	n = pm_registry_format(&e, 1, out);
	h_true(n > 0, "registre systeme formate");
	h_eq_i64("registre systeme pose", fake_fs_put(FAKE_SYSREG, text, (size_t)n),
		0);
}

static void	t_meme_signataire(void)
{
	uint8_t	cert[PM_CERT_LEN];

	fake_setup(&g_pm);
	h_eq_i64("installe", fake_install(&g_pm, "com.a", 1, 0xaa), 0);
	memset(cert, 0xaa, sizeof(cert));
	h_eq_i64("meme signataire", apkrun_signer_check(&g_pm, "com.a", cert), 0);
}

static void	t_autre_signataire(void)
{
	uint8_t	cert[PM_CERT_LEN];

	fake_setup(&g_pm);
	h_eq_i64("installe", fake_install(&g_pm, "com.a", 1, 0xaa), 0);
	memset(cert, 0xbb, sizeof(cert));
	h_eq_i64("autre signataire", apkrun_signer_check(&g_pm, "com.a", cert),
		E_ACCES);
	memset(cert, 0xaa, sizeof(cert));
	cert[PM_CERT_LEN - 1] ^= 1;
	h_eq_i64("dernier octet different",
		apkrun_signer_check(&g_pm, "com.a", cert), E_ACCES);
	memset(cert, 0xaa, sizeof(cert));
	cert[0] ^= 0x80;
	h_eq_i64("premier bit different",
		apkrun_signer_check(&g_pm, "com.a", cert), E_ACCES);
	memset(cert, 0, sizeof(cert));
	h_eq_i64("signataire nul", apkrun_signer_check(&g_pm, "com.a", cert),
		E_ACCES);
}

static void	t_systeme_autre_signataire(void)
{
	uint8_t	cert[PM_CERT_LEN];

	fake_setup(&g_pm);
	put_system_app("sys.app", 0x11);
	memset(cert, 0x11, sizeof(cert));
	h_eq_i64("systeme conforme", apkrun_signer_check(&g_pm, "sys.app", cert),
		0);
	memset(cert, 0x22, sizeof(cert));
	h_eq_i64("systeme usurpe", apkrun_signer_check(&g_pm, "sys.app", cert),
		E_ACCES);
}

static void	t_paquet_inconnu(void)
{
	uint8_t	cert[PM_CERT_LEN];

	fake_setup(&g_pm);
	h_eq_i64("installe", fake_install(&g_pm, "com.a", 1, 0xaa), 0);
	memset(cert, 0x33, sizeof(cert));
	h_eq_i64("non enregistre", apkrun_signer_check(&g_pm, "com.autre", cert),
		E_ACCES);
}

static void	t_registre_illisible(void)
{
	uint8_t	cert[PM_CERT_LEN];

	fake_setup(&g_pm);
	h_eq_i64("installe", fake_install(&g_pm, "com.a", 1, 0xaa), 0);
	memset(cert, 0xaa, sizeof(cert));
	g_fs.ops = 0;
	g_fs.fail_at = 1;
	h_true(apkrun_signer_check(&g_pm, "com.a", cert) < 0,
		"lecture en panne : refus");
	g_fs.fail_at = 0;
	fake_mem_reset(1);
	h_true(apkrun_signer_check(&g_pm, "com.a", cert) < 0,
		"memoire en panne : refus");
	fake_mem_reset(0);
	h_eq_i64("sans panne", apkrun_signer_check(&g_pm, "com.a", cert), 0);
	h_eq_i64("aucune fuite", g_mem.live, 0);
}

static void	t_arguments_invalides(void)
{
	uint8_t	cert[PM_CERT_LEN];

	fake_setup(&g_pm);
	memset(cert, 0, sizeof(cert));
	h_eq_i64("pm nul", apkrun_signer_check(NULL, "com.a", cert), E_INVAL);
	h_eq_i64("paquet nul", apkrun_signer_check(&g_pm, NULL, cert), E_INVAL);
	h_eq_i64("cert nul", apkrun_signer_check(&g_pm, "com.a", NULL), E_INVAL);
}

int	main(void)
{
	h_begin("e04 signataire au lancement");
	h_run("signataire_identique", t_meme_signataire);
	h_run("signataire_different", t_autre_signataire);
	h_run("systeme_signataire_different", t_systeme_autre_signataire);
	h_run("paquet_non_enregistre", t_paquet_inconnu);
	h_run("registre_illisible_refuse", t_registre_illisible);
	h_run("arguments_invalides", t_arguments_invalides);
	return (h_end());
}
