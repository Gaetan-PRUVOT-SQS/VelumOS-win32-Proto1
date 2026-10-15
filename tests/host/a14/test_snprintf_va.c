#include <stdarg.h>
#include <stdint.h>
#include "fake_f3.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdio.h"

static char	g_out[32];

static int	va_wrap(char *buf, size_t size, const char *fmt, ...)
{
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vsnprintf(buf, size, fmt, ap);
	va_end(ap);
	return (n);
}

static void	va_equivalence(void)
{
	char	a[64];
	char	b[64];
	int		na;
	int		nb;

	na = snprintf(a, sizeof(a), "%d|%s|%#x|%c|%lld", -9, "xy", 255, 'q', 5LL);
	nb = va_wrap(b, sizeof(b), "%d|%s|%#x|%c|%lld", -9, "xy", 255, 'q', 5LL);
	h_eq_i64("meme longueur", na, nb);
	h_eq_str("meme contenu", a, b);
	na = snprintf(a, 5, "%s", "abcdefgh");
	nb = va_wrap(b, 5, "%s", "abcdefgh");
	h_true(na == nb && na == 8, "troncature identique");
	h_eq_str("troncature : contenu", b, "abcd");
}

static void	va_noterm_call(void *arg)
{
	snprintf(g_out, sizeof(g_out), "%.3s", (const char *)arg);
}

static void	va_noterm_precision(void)
{
	char	*g;
	int		ok;

	g = fake_guarded(3);
	g[0] = 'a';
	g[1] = 'b';
	g[2] = 'c';
	ok = f3_survives(va_noterm_call, g);
	h_true(ok, "%.3s sur chaine non terminee : pas de lecture au-dela");
	fake_guarded_free(g, 3);
}

int	main(void)
{
	h_begin("a14/snprintf_va");
	h_run("vsnprintf/metamorphique : egal a snprintf", va_equivalence);
	h_run("snprintf/supposition d'erreur : chaine sans NUL avec precision",
		va_noterm_precision);
	return (h_end());
}
