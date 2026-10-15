#include "fake.h"

static void	t_package(void)
{
	char	b[200];

	h_true(fake_ok("a.b", "1", "L", "La/B;"), "paquet minimal");
	h_true(!fake_ok("", "1", "L", "La/B;"), "paquet vide");
	h_true(!fake_ok("ab", "1", "L", "La/B;"), "paquet sans point");
	h_true(!fake_ok("a..b", "1", "L", "La/B;"), "paquet point double");
	h_true(!fake_ok(".a.b", "1", "L", "La/B;"), "paquet point en tete");
	h_true(!fake_ok("a.b.", "1", "L", "La/B;"), "paquet point en fin");
	h_true(!fake_ok("a/b.c", "1", "L", "La/B;"), "paquet barre");
	h_true(!fake_ok("a b.c", "1", "L", "La/B;"), "paquet espace");
	h_true(!fake_ok("a\xc3\xa9.c", "1", "L", "La/B;"), "paquet non ascii");
	h_true(!fake_ok("a\x01.c", "1", "L", "La/B;"), "paquet controle");
	h_true(fake_ok("A_z.09", "1", "L", "La/B;"), "paquet alphabet");
	h_true(fake_ok(fake_rep(b, 125, ".b"), "1", "L", "La/B;"), "paquet 127");
	h_true(!fake_ok(fake_rep(b, 126, ".b"), "1", "L", "La/B;"), "paquet 128");
}

static void	t_version(void)
{
	h_true(fake_ok("a.b", "0", "L", "La/B;"), "version zero");
	h_true(fake_ok("a.b", "4294967295", "L", "La/B;"), "version max");
	h_true(!fake_ok("a.b", "4294967296", "L", "La/B;"), "version max plus 1");
	h_true(!fake_ok("a.b", "99999999999", "L", "La/B;"), "version 11 chiffres");
	h_true(!fake_ok("a.b", "", "L", "La/B;"), "version vide");
	h_true(!fake_ok("a.b", "01", "L", "La/B;"), "version zero en tete");
	h_true(!fake_ok("a.b", "-1", "L", "La/B;"), "version negative");
	h_true(!fake_ok("a.b", "+1", "L", "La/B;"), "version signe plus");
	h_true(!fake_ok("a.b", " 1", "L", "La/B;"), "version espace");
	h_true(!fake_ok("a.b", "1a", "L", "La/B;"), "version lettre");
	h_true(!fake_ok("a.b", "0x10", "L", "La/B;"), "version hexa");
}

static void	t_label(void)
{
	char	b[200];

	h_true(fake_ok("a.b", "1", "", "La/B;"), "libelle vide");
	h_true(fake_ok("a.b", "1", fake_rep(b, 63, ""), "La/B;"), "libelle 63");
	h_true(!fake_ok("a.b", "1", fake_rep(b, 64, ""), "La/B;"), "libelle 64");
	h_true(!fake_ok("a.b", "1", "a;b", "La/B;"), "libelle point-virgule");
	h_true(!fake_ok("a.b", "1", "a\x01", "La/B;"), "libelle controle");
	h_true(!fake_ok("a.b", "1", "a\tb", "La/B;"), "libelle tabulation");
	h_true(!fake_ok("a.b", "1", "a\x7f", "La/B;"), "libelle del");
	h_true(fake_ok("a.b", "1", "\xc3\xa9t\xc3\xa9", "La/B;"), "libelle utf8");
	h_true(fake_ok("a.b", "1", "\xf0\x9f\x98\x80", "La/B;"), "libelle 4 o");
	h_true(!fake_ok("a.b", "1", "\xc3", "La/B;"), "libelle utf8 tronque");
	h_true(!fake_ok("a.b", "1", "\xc0\xaf", "La/B;"), "libelle surlong");
	h_true(!fake_ok("a.b", "1", "\xed\xa0\x80", "La/B;"), "libelle substitut");
	h_true(!fake_ok("a.b", "1", "\xf4\x90\x80\x80", "La/B;"), "lib > 10ffff");
	h_true(!fake_ok("a.b", "1", "\x80", "La/B;"), "libelle suite seule");
}

static void	t_activity(void)
{
	char	b[200];

	h_true(fake_ok("a.b", "1", "L", "La;"), "activite minimale");
	h_true(fake_ok("a.b", "1", "L", "La/B$1_c;"), "activite alphabet");
	h_true(!fake_ok("a.b", "1", "L", "L;"), "activite trop courte");
	h_true(!fake_ok("a.b", "1", "L", ""), "activite vide");
	h_true(!fake_ok("a.b", "1", "L", "a/B;"), "activite sans L");
	h_true(!fake_ok("a.b", "1", "L", "La/B"), "activite sans point-virgule");
	h_true(!fake_ok("a.b", "1", "L", "La.B;"), "activite point");
	h_true(!fake_ok("a.b", "1", "L", "La;B;"), "activite ; interne");
	h_true(!fake_ok("a.b", "1", "L", "La B;"), "activite espace");
	fake_rep(b, 190, ";")[0] = 'L';
	h_true(fake_ok("a.b", "1", "L", b), "activite 191");
	fake_rep(b, 191, ";")[0] = 'L';
	h_true(!fake_ok("a.b", "1", "L", b), "activite 192");
}

int	main(void)
{
	h_begin("d10 champs du registre");
	h_run("champ_paquet_partitions_limites", t_package);
	h_run("champ_version_partitions_limites", t_version);
	h_run("champ_libelle_partitions_limites", t_label);
	h_run("champ_activite_partitions_limites", t_activity);
	return (h_end());
}
