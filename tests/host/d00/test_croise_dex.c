#include "harness.h"
#include "d00.h"

static void	dex_des_essais_accepte(void)
{
	t_span		file;
	t_dex		d;
	uint32_t	refused;

	file = d00_load("essais.dex");
	h_true(file.p != NULL, "essais.dex lu");
	h_eq_i64("dex_open", dex_open(&d, file), 0);
	h_eq_i64("sept classes", d.n[DEX_T_CLASS] >= 7, 1);
	h_true(d00_verify_all(&d, &refused) >= 7, "au moins sept methodes");
	h_eq_u64("aucune methode refusee", refused, 0);
	h_true(dex_find_class(&d, "Lcom/velum/essais/Arithmetique;") >= 0
		|| dex_find_class(&d, "Lessais/Arithmetique;") >= 0,
		"classe Arithmetique trouvee");
	d00_free(file);
}

static void	dex_de_bonjour_accepte(void)
{
	t_span		file;
	t_dex		d;
	uint32_t	refused;

	file = d00_load("bonjour.dex");
	h_eq_i64("dex_open", dex_open(&d, file), 0);
	h_true(d00_verify_all(&d, &refused) >= 2, "onCreate et onClick");
	h_eq_u64("aucune methode refusee", refused, 0);
	h_true(dex_find_class(&d, "Lcom/velum/bonjour/Principale;") >= 0,
		"classe principale trouvee");
	h_eq_i64("classe absente", dex_find_class(&d, "Lx/Absente;"), E_NOENT);
	d00_free(file);
}

int	main(void)
{
	h_begin("d00/croise-dex");
	h_run("dex_des_essais_accepte", dex_des_essais_accepte);
	h_run("dex_de_bonjour_accepte", dex_de_bonjour_accepte);
	return (h_end());
}
