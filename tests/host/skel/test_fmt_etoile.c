#include <limits.h>
#include "fmt_cases.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/klog.h"

static void	etoile_largeur(void)
{
	char	b[16];

	h_eq_i64("largeur 5", ksnprintf(b, sizeof(b), "%*d|", 5, 7), 6);
	h_eq_str("largeur 5 : a droite", b, "    7|");
	h_eq_i64("largeur -5", ksnprintf(b, sizeof(b), "%*d|", -5, 7), 6);
	h_eq_str("largeur -5 : drapeau moins", b, "7    |");
	h_eq_i64("largeur 0", ksnprintf(b, sizeof(b), "%*d|", 0, 7), 2);
	h_eq_str("largeur 0 : sans effet", b, "7|");
	h_eq_i64("INT_MAX", ksnprintf(b, sizeof(b), "%*d", INT_MAX, 7), INT_MAX);
	h_eq_str("INT_MAX : espaces", b, SP15);
	h_eq_i64("-INT_MAX", ksnprintf(b, sizeof(b), "%*d", -INT_MAX, 7),
		INT_MAX);
	h_eq_str("-INT_MAX : a gauche", b, "7" SP14);
	h_eq_i64("INT_MIN", ksnprintf(b, sizeof(b), "%*d", INT_MIN, 7),
		E_OVERFLOW);
	h_eq_str("INT_MIN : a gauche, tronque", b, "7" SP14);
	h_eq_i64("INT_MIN %s", ksnprintf(b, sizeof(b), "%*s", INT_MIN, "ab"),
		E_OVERFLOW);
	h_eq_str("INT_MIN %s : a gauche", b, "ab" "             ");
}

static void	etoile_precision(void)
{
	char	b[16];

	h_eq_i64("precision 3", ksnprintf(b, sizeof(b), "%.*d", 3, 7), 3);
	h_eq_str("precision 3 : zeros", b, "007");
	h_eq_i64("precision -1", ksnprintf(b, sizeof(b), "%.*d", -1, 0), 1);
	h_eq_str("precision -1 : absente", b, "0");
	h_eq_i64("INT_MIN", ksnprintf(b, sizeof(b), "%.*d", INT_MIN, 7), 1);
	h_eq_str("precision INT_MIN : absente", b, "7");
	h_eq_i64("INT_MIN zero", ksnprintf(b, sizeof(b), "%05.*d", INT_MIN, 7),
		5);
	h_eq_str("precision absente : drapeau 0 garde", b, "00007");
	h_eq_i64("INT_MAX", ksnprintf(b, sizeof(b), "%.*d", INT_MAX, 7),
		INT_MAX);
	h_eq_str("precision INT_MAX : zeros", b, ZE15);
	h_eq_i64("INT_MAX %s", ksnprintf(b, sizeof(b), "%.*s", INT_MAX, "ab"), 2);
	h_eq_str("precision INT_MAX %s : chaine entiere", b, "ab");
}

static void	etoile_croisee(void)
{
	char	b[16];

	h_eq_i64("largeur et precision INT_MAX", ksnprintf(b, sizeof(b), "%*.*d",
			INT_MAX, INT_MAX, 7), INT_MAX);
	h_eq_str("que des zeros", b, ZE15);
	h_eq_i64("INT_MIN, INT_MIN", ksnprintf(b, sizeof(b), "%*.*d", INT_MIN,
			INT_MIN, 7), E_OVERFLOW);
	h_eq_str("a gauche, precision absente", b, "7" SP14);
	h_eq_i64("apres une erreur", ksnprintf(b, sizeof(b), "%d", 42), 2);
	h_eq_str("appel suivant intact", b, "42");
}

int	main(void)
{
	h_begin("skel/fmt_etoile");
	h_run("I-F3-3 largeur par etoile", etoile_largeur);
	h_run("I-F3-3 precision par etoile", etoile_precision);
	h_run("I-F3-3 largeur et precision par etoile", etoile_croisee);
	return (h_end());
}
