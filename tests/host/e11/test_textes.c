#include "harness.h"
#include "velum/libk.h"
#include "velum/err.h"
#include "fichiers_model.h"

static void	taille(uint64_t v, const char *want)
{
	char	out[FF_SIZE_TEXT_MAX];

	h_eq_i64(want, ff_size_text(out, sizeof(out), v), (int64_t)strlen(want));
	h_eq_str(want, out, want);
}

static void	tailles_valeurs_limites(void)
{
	taille(0, "0 o");
	taille(1023, "1023 o");
	taille(1024, "1,0 Ko");
	taille(1536, "1,5 Ko");
	taille(1048575, "1023,9 Ko");
	taille(1048576, "1,0 Mo");
	taille(25794970, "24,6 Mo");
	taille(5905580032ull, "5,5 Go");
	taille(1099511627776ull, "1,0 To");
	taille(9223372036854775808ull, "8388608,0 To");
}

static void	taille_tampon_trop_petit(void)
{
	char	out[4];

	h_eq_i64("débordement signalé", ff_size_text(out, sizeof(out), 1536),
		E_RANGE);
}

static void	compteur_maquette(void)
{
	char	out[FF_SIZE_TEXT_MAX];

	ff_count_text(out, sizeof(out), 7);
	h_eq_str("maquette 04 : 7 éléments", out, "7 éléments");
	ff_count_text(out, sizeof(out), 1);
	h_eq_str("singulier", out, "1 élément");
	ff_count_text(out, sizeof(out), 0);
	h_eq_str("zéro", out, "0 élément");
}

static void	messages_d_erreur(void)
{
	h_eq_str("existe", ff_error_text(E_EXIST), "Ce nom existe déjà.");
	h_eq_str("non vide", ff_error_text(E_NOTEMPTY),
		"Le dossier n'est pas vide.");
	h_true(ff_error_text(-9999)[0] != '\0', "code inconnu : message non vide");
}

int	main(void)
{
	h_begin("e11/textes");
	h_run("tailles: valeurs limites", tailles_valeurs_limites);
	h_run("tailles: tampon trop petit", taille_tampon_trop_petit);
	h_run("compteur: maquette 04", compteur_maquette);
	h_run("erreurs: messages", messages_d_erreur);
	return (h_end());
}
