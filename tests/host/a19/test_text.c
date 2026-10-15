#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	strip_cases(void)
{
	char	b[CTL_TEXT_MAX];
	int32_t	mn;

	h_eq_i64("longueur", ctl_text_strip("&Fichier", b, &mn), 7);
	h_eq_str("texte", b, "Fichier");
	h_eq_i64("mnemonique en tete", mn, 0);
	ctl_text_strip("Ou&vrir", b, &mn);
	h_eq_i64("mnemonique au milieu", mn, 2);
	ctl_text_strip("A&&B", b, &mn);
	h_eq_str("&& litteral", b, "A&B");
	h_eq_i64("pas de mnemonique avec &&", mn, -1);
	ctl_text_strip("a&b&c", b, &mn);
	h_eq_str("deux marqueurs", b, "abc");
	h_eq_i64("seul le premier compte", mn, 1);
	ctl_text_strip("fin&", b, &mn);
	h_eq_str("& final ignore", b, "fin");
	h_eq_i64("pas de mnemonique final", mn, -1);
	ctl_text_strip("", b, &mn);
	h_eq_str("vide", b, "");
	ctl_text_strip(NULL, b, &mn);
	h_eq_str("NULL", b, "");
}

static void	mnemonic_chars(void)
{
	h_eq_i64("lettre majuscule", ctl_mnemonic("&Ok"), 'O');
	h_eq_i64("lettre minuscule", ctl_mnemonic("a&nnuler"), 'N');
	h_eq_i64("chiffre", ctl_mnemonic("&1 un"), '1');
	h_eq_i64("accent non ascii", ctl_mnemonic("&\xc3\xa9t\xc3\xa9"), 0);
	h_eq_i64("ponctuation", ctl_mnemonic("&.x"), 0);
	h_eq_i64("sans marqueur", ctl_mnemonic("Ok"), 0);
}

static void	copy_bounds(void)
{
	char	big[400];
	char	out[CTL_TEXT_MAX];

	memset(big, 'a', sizeof(big));
	big[399] = '\0';
	ctl_copy_text(out, big);
	h_eq_i64("255 octets au maximum", (int64_t)strlen(out), 255);
	memcpy(big + 254, "\xe2\x82\xac", 3);
	big[257] = '\0';
	ctl_copy_text(out, big);
	h_eq_i64("euro a cheval refuse", (int64_t)strlen(out), 254);
	ctl_copy_text(out, NULL);
	h_eq_str("NULL donne vide", out, "");
	ctl_copy_text(out, "ab");
	ctl_copy_text(out, out);
	h_eq_str("source = destination", out, "ab");
}

int	main(void)
{
	h_begin("a19/text");
	h_run("retrait des & : partitions", strip_cases);
	h_run("mnemonique : lettre, chiffre, non ascii", mnemonic_chars);
	h_run("copie de texte : bornes et frontieres utf-8", copy_bounds);
	return (h_end());
}
