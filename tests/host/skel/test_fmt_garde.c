#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "harness.h"
#include "velum/klog.h"

static char	*garde_colle(const char *src, size_t n)
{
	size_t	page;
	char	*map;

	page = (size_t)sysconf(_SC_PAGESIZE);
	map = mmap(NULL, 2 * page, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (map == MAP_FAILED || mprotect(map + page, page, PROT_NONE))
	{
		h_true(0, "page de garde posee");
		return (NULL);
	}
	memcpy(map + page - n, src, n);
	return (map + page - n);
}

static void	garde_sans_nul(void)
{
	char	b[32];
	char	*s;

	s = garde_colle("abc", 3);
	if (!s)
		return ;
	h_eq_i64("precision 0", ksnprintf(b, sizeof(b), "%.0s", s + 3), 0);
	h_eq_str("precision 0 : vide", b, "");
	h_eq_i64("precision 1", ksnprintf(b, sizeof(b), "%.1s", s + 2), 1);
	h_eq_str("precision 1 : dernier octet", b, "c");
	h_eq_i64("precision exacte", ksnprintf(b, sizeof(b), "%.3s", s), 3);
	h_eq_str("precision exacte : abc", b, "abc");
	h_eq_i64("precision courte", ksnprintf(b, sizeof(b), "%.2s", s), 2);
	h_eq_str("precision courte : ab", b, "ab");
	h_eq_i64("largeur", ksnprintf(b, sizeof(b), "%5.3s|%-5.3s|", s, s), 12);
	h_eq_str("largeur et precision", b, "  abc|abc  |");
	h_eq_i64("etoile", ksnprintf(b, sizeof(b), "%.*s", 3, s), 3);
	h_eq_str("precision par etoile", b, "abc");
}

static void	garde_avec_nul(void)
{
	char	b[32];
	char	*s;

	s = garde_colle("ab", 3);
	if (!s)
		return ;
	h_eq_i64("precision longue", ksnprintf(b, sizeof(b), "%.10s", s), 2);
	h_eq_str("precision plus longue que la chaine", b, "ab");
	h_eq_i64("precision absente", ksnprintf(b, sizeof(b), "%s", s), 2);
	h_eq_str("precision absente : jusqu'au NUL", b, "ab");
	h_eq_i64("etoile negative", ksnprintf(b, sizeof(b), "%.*s", -1, s), 2);
	h_eq_str("precision negative : absente", b, "ab");
	h_eq_i64("enorme", ksnprintf(b, sizeof(b), "%.99999999999s", s), 2);
	h_eq_str("precision enorme : jusqu'au NUL", b, "ab");
}

int	main(void)
{
	h_begin("skel/fmt_garde");
	h_run("I-F3-1 sans NUL contre une page interdite", garde_sans_nul);
	h_run("I-F3-1 avec NUL en fin de page", garde_avec_nul);
	return (h_end());
}
