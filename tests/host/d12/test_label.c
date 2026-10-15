#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "velum/err.h"
#include "apkglue.h"
#include "fake.h"

static void	label_court(void)
{
	char	dst[64];

	memset(dst, 'x', sizeof(dst));
	h_eq_u64("ascii", apkglue_label(dst, sizeof(dst), "Bonjour"), 7);
	h_eq_str("copie", dst, "Bonjour");
	h_eq_u64("vide", apkglue_label(dst, sizeof(dst), ""), 0);
	h_eq_str("vide", dst, "");
	h_eq_u64("capacite nulle", apkglue_label(dst, 0, "abc"), 0);
	h_eq_u64("capacite un", apkglue_label(dst, 1, "abc"), 0);
	h_eq_str("capacite un", dst, "");
	h_eq_u64("juste", apkglue_label(dst, 4, "abc"), 3);
	h_eq_u64("coupe ascii", apkglue_label(dst, 3, "abc"), 2);
	h_eq_str("coupe ascii", dst, "ab");
}

static void	label_coupe_utf8(void)
{
	char	dst[64];

	h_eq_u64("2 octets", apkglue_label(dst, 4, "ab\xc3\xa9" "c"), 2);
	h_eq_str("2 octets", dst, "ab");
	h_eq_u64("2 octets entier", apkglue_label(dst, 5, "ab\xc3\xa9" "c"), 4);
	h_eq_u64("3 octets", apkglue_label(dst, 4, "a\xe2\x82\xac" "b"), 1);
	h_eq_u64("3 octets", apkglue_label(dst, 5, "a\xe2\x82\xac" "b"), 4);
	h_eq_u64("4 octets", apkglue_label(dst, 5, "a\xf0\x9f\x98\x80"), 1);
	h_eq_u64("4 octets", apkglue_label(dst, 6, "a\xf0\x9f\x98\x80"), 5);
	h_eq_u64("suite seule", apkglue_label(dst, 3, "\x80\x80\x80\x80"), 0);
}

static void	label_soixante_trois(void)
{
	char	src[130];
	char	dst[64];
	int		i;

	i = 0;
	while (i < 64)
	{
		src[2 * i] = (char)0xc3;
		src[2 * i + 1] = (char)0xa9;
		i++;
	}
	src[128] = '\0';
	h_eq_u64("63 octets demandes, 62 gardes", apkglue_label(dst, 64, src), 62);
	h_eq_u64("longueur", strlen(dst), 62);
	memmove(src + 1, src, 129);
	src[0] = 'a';
	h_eq_u64("63 octets pleins", apkglue_label(dst, 64, src), 63);
	h_true((unsigned char)dst[62] == 0xa9, "dernier caractere entier");
}

static void	fichier_entier(void)
{
	uint8_t	*buf;

	fv_reset();
	h_eq_i64("absent", apkglue_read_file("/d/a", &buf), E_NOENT);
	h_true(buf == NULL, "rien rendu");
	fv_put("/d/a", "un fichier de vingt-six oct");
	h_eq_i64("lu", apkglue_read_file("/d/a", &buf), 27);
	h_true(buf && memcmp(buf, "un fichier de vingt-six oct", 27) == 0, "lu");
	free(buf);
	fv_put("/d/v", "");
	h_eq_i64("vide", apkglue_read_file("/d/v", &buf), 0);
	free(buf);
	g_fv.fail_at = g_fv.calls + 1;
	h_eq_i64("panne", apkglue_read_file("/d/a", &buf), E_IO);
	h_true(buf == NULL, "rien rendu apres panne");
}

int	main(void)
{
	h_begin("d12 libelle et lecture");
	h_run("label_court", label_court);
	h_run("label_coupe_utf8", label_coupe_utf8);
	h_run("label_soixante_trois", label_soixante_trois);
	h_run("fichier_entier", fichier_entier);
	return (h_end());
}
