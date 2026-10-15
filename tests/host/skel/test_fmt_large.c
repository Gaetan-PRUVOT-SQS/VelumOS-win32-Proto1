#include <limits.h>
#include <time.h>
#include "fmt_cases.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/klog.h"

static const t_bigcase	g_big[16] = {
{"%2147483646d", 7, INT_MAX - 1, SP15},
{"%2147483647d", 7, INT_MAX, SP15},
{"%-2147483647d", 7, INT_MAX, "7" SP14},
{"%02147483647d", 7, INT_MAX, ZE15},
{"%2147483647c", 'z', INT_MAX, SP15},
{"%.2147483647d", 7, INT_MAX, ZE15},
{"%.2147483646d", -7, INT_MAX, "-" ZE14},
{"%2147483648d", 7, E_OVERFLOW, SP15},
{"%99999999999d", 7, E_OVERFLOW, SP15},
{"%-99999999999999999999999d", 7, E_OVERFLOW, "7" SP14},
{"%.2147483647d", -7, E_OVERFLOW, "-" ZE14},
{"%.2147483648d", 7, E_OVERFLOW, ZE15},
{"%.99999999999d", 7, E_OVERFLOW, ZE15},
{"%2147483647d|", 7, E_OVERFLOW, SP15},
{"%2147483647d%2147483647d", 7, E_OVERFLOW, SP15},
{"%99999999999.99999999999x", 7, E_OVERFLOW, ZE15}
};

static void	large_temps(void)
{
	char	b[16];
	clock_t	debut;
	int		appels;

	debut = clock();
	appels = 0;
	while (appels < BUDGET_APPELS
		&& clock() - debut < BUDGET_SECONDES * CLOCKS_PER_SEC)
	{
		ksnprintf(b, sizeof(b), "%2147483647d|%.2147483647d", 7, 7);
		appels++;
	}
	h_eq_i64("1000 remplissages enormes dans le budget", appels,
		BUDGET_APPELS);
	h_true(clock() - debut < BUDGET_SECONDES * CLOCKS_PER_SEC,
		"temps borne par la capacite du tampon");
}

static void	large_table(void)
{
	char	b[16];
	int		i;

	i = 0;
	while (i < 16)
	{
		h_eq_i64(g_big[i].fmt, ksnprintf(b, sizeof(b), g_big[i].fmt,
				g_big[i].arg, g_big[i].arg), g_big[i].len);
		h_eq_str("contenu tronque et termine", b, g_big[i].want);
		i++;
	}
}

static void	large_sans_tampon(void)
{
	char	b[2];

	b[0] = 'x';
	b[1] = 'y';
	h_eq_i64("taille 0", ksnprintf(b, 0, "%2147483647d", 7), INT_MAX);
	h_eq_i64("taille 0 : rien d'ecrit", b[0], 'x');
	h_eq_i64("taille 1", ksnprintf(b, 1, "%2147483647d", 7), INT_MAX);
	h_eq_i64("taille 1 : NUL seul", b[0], 0);
	h_eq_i64("taille 1 : pas de debordement", b[1], 'y');
	h_eq_i64("taille 0 depassee", ksnprintf(b, 0, "%99999999999d", 7),
		E_OVERFLOW);
}

int	main(void)
{
	h_begin("skel/fmt_large");
	h_run("I-F3-3 temps borne", large_temps);
	h_run("I-F3-3 limites de largeur et de precision", large_table);
	h_run("I-F3-3 tampon nul ou d'un octet", large_sans_tampon);
	return (h_end());
}
