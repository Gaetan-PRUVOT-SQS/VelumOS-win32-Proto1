#include "harness.h"
#include "fake.h"
#include "velum/err.h"

#define PAGE 0x1000ull
#define LIMITE 0x1000000ull
#define PAGES_LIBRES(n) ((LIMITE / PAGE) - (n))

static void	sans_limite(void)
{
	ft_reset(0, 1ull << 30);
	h_eq_i64("limite nulle : pas de plafond", ft_create(0), 42);
	h_eq_i64("pile allouee", g_ft.allocs, 1);
}

static void	pile_par_defaut_aux_bornes(void)
{
	ft_reset(LIMITE, PAGES_LIBRES(TSTACK_DEFAULT / PAGE));
	h_eq_i64("ajustement exact : accepte", ft_create(0), 42);
	ft_reset(LIMITE, PAGES_LIBRES(TSTACK_DEFAULT / PAGE) + 1);
	h_eq_i64("une page de trop : refuse", ft_create(0), E_NOMEM);
	h_eq_i64("rien n'a ete mappe", g_ft.allocs, 0);
	h_eq_i64("rien a defaire", g_ft.unmaps, 0);
}

static void	taille_explicite(void)
{
	ft_reset(LIMITE, PAGES_LIBRES(0x10000 / PAGE));
	h_eq_i64("64 Kio tient", ft_create(0x10000), 42);
	h_eq_i64("128 Kio ne tient pas", ft_create(0x20000), E_NOMEM);
	ft_reset(LIMITE, PAGES_LIBRES(0x5000 / PAGE));
	h_eq_i64("0x4001 arrondi a 0x5000 : tient", ft_create(0x4001), 42);
	ft_reset(LIMITE, PAGES_LIBRES(0x5000 / PAGE) + 1);
	h_eq_i64("0x4001 arrondi a 0x5000 : une page de trop", ft_create(0x4001),
		E_NOMEM);
}

static void	tailles_invalides_gardent_leur_code(void)
{
	ft_reset(LIMITE, LIMITE / PAGE);
	h_eq_i64("trop petite : E_INVAL", ft_create(0x1000), E_INVAL);
	h_eq_i64("trop grande : E_INVAL", ft_create(TSTACK_MAX + 1), E_INVAL);
	h_eq_i64("valeur geante : E_INVAL", ft_create(~0ull), E_INVAL);
}

static void	echec_de_creation_rend_la_pile(void)
{
	ft_reset(LIMITE, 0);
	g_ft.fail_add = 1;
	h_eq_i64("fil non cree", ft_create(0), E_AGAIN);
	h_eq_i64("pile rendue", g_ft.unmaps, 1);
}

int	main(void)
{
	h_begin("e05t/thread_limite");
	h_run("sans limite", sans_limite);
	h_run("pile par defaut aux bornes", pile_par_defaut_aux_bornes);
	h_run("taille explicite", taille_explicite);
	h_run("tailles invalides", tailles_invalides_gardent_leur_code);
	h_run("echec de creation", echec_de_creation_rend_la_pile);
	return (h_end());
}
