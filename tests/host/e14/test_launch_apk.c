#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "sh_launch_apk.h"

#define CERT "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff"

static t_launchapks	g_a;

static int	analyse(const char *txt, uint32_t origin)
{
	memset(&g_a, 0, sizeof(g_a));
	return (sh_launch_apk_parse(txt, strlen(txt), origin, &g_a));
}

static void	lignes_valides(void)
{
	h_eq_i64("deux lignes",
		analyse("com.velum.bonjour;1;Bonjour;.Main;" CERT "\n"
			"org.exemple.notes;12;Notes \xc3\xa9tendues;.Notes;" CERT "\n",
			SH_LAUNCH_ORIGIN_DATA), 2);
	h_eq_str("paquet 0", g_a.list[0].package, "com.velum.bonjour");
	h_eq_str("etiquette 0", g_a.list[0].label, "Bonjour");
	h_eq_str("etiquette UTF-8", g_a.list[1].label, "Notes \xc3\xa9tendues");
	h_eq_i64("origine", g_a.list[1].origin, SH_LAUNCH_ORIGIN_DATA);
	h_eq_i64("sans retour final", analyse("a.b;1;X;.A;" CERT, 0), 1);
	h_eq_i64("fin de ligne CRLF", analyse("a.b;1;X;.A;" CERT "\r\n", 0), 1);
	h_eq_str("CRLF : pas de CR dans le paquet", g_a.list[0].package, "a.b");
}

static void	lignes_refusees(void)
{
	h_eq_i64("paquet vide", analyse(";1;X;.A;" CERT "\n", 0), 0);
	h_eq_i64("traversee ..", analyse("a..b;1;X;.A;" CERT "\n", 0), 0);
	h_eq_i64("slash dans le paquet", analyse("a/b;1;X;.A;" CERT "\n", 0), 0);
	h_eq_i64("point initial", analyse(".ab;1;X;.A;" CERT "\n", 0), 0);
	h_eq_i64("point final", analyse("ab.;1;X;.A;" CERT "\n", 0), 0);
	h_eq_i64("etiquette vide", analyse("a.b;1;;.A;" CERT "\n", 0), 0);
	h_eq_i64("etiquette avec controle", analyse("a.b;1;X\x01;.A;" CERT "\n", 0), 0);
	h_eq_i64("champs manquants", analyse("a.b;1\n", 0), 0);
	h_eq_i64("lignes vides", analyse("\n\n\n", 0), 0);
	h_eq_i64("doublon ignore", analyse("a.b;1;X;.A;" CERT "\na.b;2;Y;.A;" CERT "\n",
			0), 1);
	h_eq_str("premier gagne", g_a.list[0].label, "X");
}

static void	bornes(void)
{
	char		txt[8192];
	size_t		n;
	uint32_t	i;

	n = 0;
	i = 0;
	while (i < SH_LAUNCH_APKS_MAX + 5)
	{
		n += (size_t)snprintf(txt + n, sizeof(txt) - n,
			"p.n%u;1;App %u;.A;" CERT "\n", i, i);
		i++;
	}
	h_eq_i64("capacite", analyse(txt, 1), SH_LAUNCH_APKS_MAX);
	memset(txt, 'a', 200);
	memcpy(txt + 200, ";1;X;.A;" CERT "\n", strlen(";1;X;.A;" CERT "\n") + 1);
	h_eq_i64("paquet trop long", analyse(txt, 0), 0);
	h_eq_i64("etiquette tronquee proprement",
		analyse("a.b;1;0123456789012345678901234567890123456789012345\xc3\xa9"
			"zz;.A;" CERT "\n", 0), 1);
	h_true(strlen(g_a.list[0].label) < LUNA_LM_LABEL_MAX, "etiquette bornee");
	h_eq_i64("coupe avant l'octet de tete", (uint8_t)g_a.list[0].label[45], '5');
	h_true(sh_launch_apk_parse(NULL, 5, 0, &g_a) < 0, "tampon NULL");
	h_true(sh_launch_apk_parse(NULL, 0, 0, NULL) < 0, "sortie NULL");
}

static void	chemins(void)
{
	char	p[64];

	analyse("a.b;1;X;.A;" CERT "\n", SH_LAUNCH_ORIGIN_DATA);
	h_eq_i64("chemin donnees", sh_launch_apk_path(&g_a.list[0], p, sizeof(p)), 0);
	h_eq_str("racine donnees", p, "/data/apps/a.b.apk");
	g_a.list[0].origin = SH_LAUNCH_ORIGIN_SYSTEM;
	sh_launch_apk_path(&g_a.list[0], p, sizeof(p));
	h_eq_str("racine systeme", p, "/system/apps/a.b.apk");
	h_true(sh_launch_apk_path(&g_a.list[0], p, 20) < 0, "tampon trop court");
	h_eq_i64("chaine videe en cas d'echec", p[0], 0);
	h_true(sh_launch_apk_path(NULL, p, sizeof(p)) < 0, "paquet NULL");
}

int	main(void)
{
	h_begin("e14/lanceur_apk");
	h_run("registre : lignes valides", lignes_valides);
	h_run("registre : lignes refusees", lignes_refusees);
	h_run("registre : bornes", bornes);
	h_run("chemin de l'APK", chemins);
	return (h_end());
}
