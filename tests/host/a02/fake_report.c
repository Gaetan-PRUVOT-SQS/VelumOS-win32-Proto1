#include <stdio.h>
#include "a02_fake.h"

void	fake_report(const char *label, uint64_t scanned, uint64_t calls)
{
	printf("complexite %-22s : %8llu mots lus pour %6llu appels"
		" (%llu.%02llu par appel), bitmap de %llu mots\n", label,
		(unsigned long long)scanned, (unsigned long long)calls,
		(unsigned long long)(scanned / calls),
		(unsigned long long)(scanned * 100 / calls % 100),
		(unsigned long long)g_pmm.words);
}
