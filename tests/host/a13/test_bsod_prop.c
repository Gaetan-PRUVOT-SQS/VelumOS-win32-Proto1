#include <stdio.h>
#include "kfix.h"

static void	cut_cases(void)
{
	const char	*s;

	fake_font_reset();
	s = "a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80x";
	h_eq_i64("0 cellule", bsod_cut(s, 11, 0), 0);
	h_eq_i64("1 cellule", bsod_cut(s, 11, 1), 1);
	h_eq_i64("2 cellules", bsod_cut(s, 11, 2), 3);
	h_eq_i64("3 cellules", bsod_cut(s, 11, 3), 6);
	h_eq_i64("4 cellules", bsod_cut(s, 11, 4), 10);
	h_eq_i64("5 cellules", bsod_cut(s, 11, 5), 11);
	h_eq_i64("au-dela", bsod_cut(s, 11, 99), 11);
	h_eq_i64("negatif", bsod_cut(s, 11, -4), 0);
	h_eq_i64("longueur nulle", bsod_cut(s, 0, 3), 0);
	h_true(bsod_cut(s, 2, 2) <= 2, "coupe au milieu d'un caractere : bornee");
}

static void	wrap_properties(void)
{
	t_bsod_wrap	w;
	char		buf[256];
	uint64_t	seed;
	int			i;
	int			bad;

	seed = 20261005;
	printf("a13/bsod_prop : graine %llu\n", (unsigned long long)seed);
	fake_font_reset();
	i = 0;
	bad = 0;
	while (i++ < 5000)
	{
		fake_text_gen(buf, sizeof(buf), &seed);
		w.text = buf;
		w.cols = 1 + (int32_t)(kfix_rnd(&seed) % 20);
		w.max = 1 + (int32_t)(kfix_rnd(&seed) % 12);
		bsod_wrap(&w);
		bad += fake_wrapck(&w, buf, w.cols);
		bad += fake_wrapu8(&w);
	}
	h_eq_i64("proprietes sur 5000 textes", bad, 0);
}

static void	hash_vectors(void)
{
	h_eq_u64("FNV-1a de la chaine vide", bsod_hash(""), 0x811c9dc5);
	h_eq_u64("FNV-1a de a", bsod_hash("a"), 0xe40c292c);
	h_eq_u64("FNV-1a de foobar", bsod_hash("foobar"), 0xbf9cf968);
}

int	main(void)
{
	h_begin("a13/bsod_prop");
	h_run("coupe en cellules", cut_cases);
	h_run("proprietes du decoupage aleatoire", wrap_properties);
	h_run("somme du code d'arret", hash_vectors);
	return (h_end());
}
