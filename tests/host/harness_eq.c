#include <stdio.h>
#include <string.h>
#include "harness.h"

void	h_eq_i64(const char *what, int64_t got, int64_t want)
{
	h_true(got == want, what);
	if (got != want)
		fprintf(stderr, "  obtenu %lld attendu %lld\n", (long long)got,
			(long long)want);
}

void	h_eq_u64(const char *what, uint64_t got, uint64_t want)
{
	h_true(got == want, what);
	if (got != want)
		fprintf(stderr, "  obtenu %llu attendu %llu\n",
			(unsigned long long)got, (unsigned long long)want);
}

void	h_eq_str(const char *what, const char *got, const char *want)
{
	int	same;

	same = got && want && !strcmp(got, want);
	h_true(same, what);
	if (!same)
		fprintf(stderr, "  obtenu '%s' attendu '%s'\n", got, want);
}
