#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "velum/err.h"
#include "apkglue.h"
#include "fake.h"

static t_span	load(const char *name)
{
	char	path[512];
	t_span	s;
	FILE	*f;
	uint8_t	*buf;

	s = (t_span){NULL, 0};
	snprintf(path, sizeof(path), "%s/%s", D12_FIX, name);
	f = fopen(path, "rb");
	buf = malloc(1 << 20);
	if (f && buf)
	{
		s.len = fread(buf, 1, 1 << 20, f);
		s.p = buf;
	}
	else
		free(buf);
	if (f)
		fclose(f);
	h_true(s.p != NULL && s.len > 0, "fixture lue");
	return (s);
}

static int	intact(const t_pminfo *info)
{
	const uint8_t	*p;
	size_t			i;

	p = (const uint8_t *)info;
	i = 0;
	while (i < sizeof(*info) && p[i] == 0x5a)
		i++;
	return (i == sizeof(*info));
}

static void	inspect_valide(void)
{
	t_pminfo	info;
	t_span		apk;

	apk = load("ok.apk");
	memset(&info, 0x5a, sizeof(info));
	h_eq_i64("accepte", apkglue_inspect(apk, &info), 0);
	h_eq_i64("raison", apkglue_reason(), APKR_OK);
	h_eq_str("paquet", info.package, "com.velum.bonjour");
	h_eq_str("libelle", info.label, "Bonjour APK");
	h_eq_str("activite", info.activity, "Lcom/velum/bonjour/Principale;");
	h_eq_u64("version", info.version_code, 1);
	h_true(info.cert[0] != 0x5a || info.cert[31] != 0x5a, "certificat pose");
	free((void *)apk.p);
}

static void	inspect_refus(void)
{
	t_pminfo	info;
	t_span		apk;
	size_t		n;

	apk = load("nosig.apk");
	memset(&info, 0x5a, sizeof(info));
	h_true(apkglue_inspect(apk, &info) < 0, "sans signature : refuse");
	h_true(intact(&info), "sans signature : rien rendu");
	h_true(apkglue_reason() > APKR_OK, "sans signature : raison posee");
	free((void *)apk.p);
	apk = load("ok.apk");
	((uint8_t *)apk.p)[apk.len / 3] ^= 0x01;
	h_true(apkglue_inspect(apk, &info) < 0, "octet altere : refuse");
	h_true(intact(&info), "octet altere : rien rendu");
	((uint8_t *)apk.p)[apk.len / 3] ^= 0x01;
	n = 0;
	while (n < apk.len)
	{
		apk.len -= 1 + apk.len / 64;
		h_true(apkglue_inspect(apk, &info) < 0 && intact(&info), "tronque");
		n++;
	}
	free((void *)apk.p);
}

int	main(void)
{
	h_begin("d12 examen de l'APK");
	h_run("inspect_valide", inspect_valide);
	h_run("inspect_refus", inspect_refus);
	return (h_end());
}
