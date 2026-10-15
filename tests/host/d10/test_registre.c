#include <stdio.h>
#include "fake.h"

static t_fuzz	g_z;

static int	scan(const char *text, uint32_t max, uint32_t *rejected)
{
	t_pmreg	r;
	t_span	s;
	int		n;

	s.p = (const uint8_t *)text;
	s.len = strlen(text);
	r.e = g_z.e;
	r.max = max;
	n = pm_registry_scan(s, &r);
	*rejected = r.rejected;
	return (n);
}

static void	t_lines(void)
{
	uint32_t	bad;

	snprintf(g_z.text, sizeof(g_z.text), "a.b;7;L;La/B;;%s\r\n\r\n\n"
		"mauvaise ligne\nc.d;2;M;Lc/D;;%s", fake_hex(), fake_hex());
	h_eq_i64("crlf, vides, fin sans saut", scan(g_z.text, 8, &bad), 2);
	h_eq_u64("une ligne refusee", bad, 1);
	h_eq_str("paquet 0", g_z.e[0].info.package, "a.b");
	h_eq_u64("version 0", g_z.e[0].info.version_code, 7);
	h_eq_str("libelle 0", g_z.e[0].info.label, "L");
	h_eq_str("activite 0", g_z.e[0].info.activity, "La/B;");
	h_eq_u64("certificat 0", g_z.e[0].info.cert[0], 0x01);
	h_eq_u64("certificat 7", g_z.e[0].info.cert[7], 0xef);
	h_eq_str("paquet 1", g_z.e[1].info.package, "c.d");
	h_eq_i64("texte vide", scan("", 8, &bad), 0);
	h_eq_i64("que des sauts", scan("\n\r\n\n", 8, &bad), 0);
	h_eq_u64("aucun refus", bad, 0);
	h_eq_i64("binaire", scan("a.b;1;\x01\xff\n;;;;\n", 8, &bad), 0);
	h_eq_u64("deux refus", bad, 2);
}

static void	t_dup_max(void)
{
	uint32_t	bad;

	snprintf(g_z.text, sizeof(g_z.text), "a.b;1;L;La/B;;%s\na.b;2;L;La/B;;%s\n"
		"A.B;3;L;La/B;;%s\nc.d;4;L;La/B;;%s\ne.f;5;L;La/B;;%s\n", fake_hex(),
		fake_hex(), fake_hex(), fake_hex(), fake_hex());
	h_eq_i64("doublons ecartes", scan(g_z.text, 8, &bad), 3);
	h_eq_u64("doublon exact et de casse refuses", bad, 2);
	h_eq_u64("la premiere gagne", g_z.e[0].info.version_code, 1);
	h_eq_i64("plafond de sortie", scan(g_z.text, 2, &bad), 2);
	h_eq_u64("au-dela du plafond compte", bad, 3);
	h_eq_i64("plafond nul", scan(g_z.text, 0, &bad), 0);
	snprintf(g_z.text, sizeof(g_z.text), "a.b;1;L;La/B;;%.63s\n"
		"c.d;1;L;La/B;;%sa\ne.f;1;L;La/B;;g%.63s\n", fake_hex(), fake_hex(),
		fake_hex());
	h_eq_i64("certificat 63, 65, non hexa", scan(g_z.text, 8, &bad), 0);
	h_eq_u64("trois refus", bad, 3);
}

static void	t_roundtrip(void)
{
	static t_pmentry	back[8];
	t_text				out;
	t_span				s;
	int					len;

	len = fake_base(g_z.text, sizeof(g_z.text));
	s.p = (const uint8_t *)g_z.text;
	s.len = (size_t)len;
	h_eq_i64("base lue", pm_registry_parse(s, g_z.e, 8), 3);
	out.p = (char *)g_z.base;
	out.cap = sizeof(g_z.base);
	len = pm_registry_format(g_z.e, 3, out);
	h_true(len > 0, "format");
	s.p = g_z.base;
	s.len = (size_t)len;
	h_eq_i64("relu", pm_registry_parse(s, back, 8), 3);
	h_true(memcmp(back, g_z.e, 3 * sizeof(back[0])) == 0, "relu identique");
	out.cap = (size_t)len;
	h_eq_i64("trop court", pm_registry_format(g_z.e, 3, out), E_OVERFLOW);
	out.cap = (size_t)len + 1;
	h_eq_i64("tampon juste", pm_registry_format(g_z.e, 3, out), len);
	g_z.e[1].info.label[0] = ';';
	h_eq_i64("entree invalide", pm_registry_format(g_z.e, 3, out), E_INVAL);
}

int	main(void)
{
	h_begin("d10 registre");
	h_run("registre_lignes_crlf_vides_refus", t_lines);
	h_run("registre_doublons_plafond_certificat", t_dup_max);
	h_run("registre_aller_retour_format_analyse", t_roundtrip);
	return (h_end());
}
