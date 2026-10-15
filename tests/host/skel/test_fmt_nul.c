#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include "harness.h"
#include "velum/err.h"
#include "velum/klog.h"

static int	nul_va(char *buf, size_t size, const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = kvsnprintf(buf, size, fmt, ap);
	va_end(ap);
	return (n);
}

static void	nul_compte(void)
{
	h_eq_i64("ksnprintf NULL, 16", ksnprintf(NULL, 16, "abc %d", 42), 6);
	h_eq_i64("kvsnprintf NULL, 16", nul_va(NULL, 16, "abc %d", 42), 6);
	h_eq_i64("NULL, 0", ksnprintf(NULL, 0, "abc %d", 42), 6);
	h_eq_i64("NULL, 1 : terminaison", ksnprintf(NULL, 1, "abc"), 3);
	h_eq_i64("NULL, 1 : format vide", ksnprintf(NULL, 1, ""), 0);
	h_eq_i64("NULL, SIZE_MAX", ksnprintf(NULL, SIZE_MAX, "%s", "abcd"), 4);
}

static void	nul_remplissage(void)
{
	h_eq_i64("largeur", ksnprintf(NULL, 16, "%10d|%-10s|", 42, "ab"), 22);
	h_eq_i64("zeros", ksnprintf(NULL, 16, "%.8d|%08x", 42, 42), 17);
	h_eq_i64("caractere", ksnprintf(NULL, 16, "%5c", 'z'), 5);
	h_eq_i64("largeur INT_MAX", ksnprintf(NULL, 16, "%2147483647d", 7),
		INT_MAX);
	h_eq_i64("depassement", ksnprintf(NULL, 16, "%2147483648d", 7),
		E_OVERFLOW);
}

static void	nul_voisins(void)
{
	char	b[8];

	b[0] = 'x';
	h_eq_i64("tampon reel, taille 0", ksnprintf(b, 0, "abc %d", 42), 6);
	h_eq_i64("taille 0 : rien d'ecrit", b[0], 'x');
	h_eq_i64("tampon reel", ksnprintf(b, sizeof(b), "abc %d", 42), 6);
	h_eq_str("tampon reel : contenu", b, "abc 42");
	ksnprintf(NULL, 16, "zzzzzzzz");
	h_eq_str("appel NULL sans effet sur un autre tampon", b, "abc 42");
}

int	main(void)
{
	h_begin("skel/fmt_nul");
	h_run("tampon NULL : longueur comptee, rien d'ecrit", nul_compte);
	h_run("tampon NULL : remplissages", nul_remplissage);
	h_run("tampon NULL : voisins avec tampon reel", nul_voisins);
	return (h_end());
}
