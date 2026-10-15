#include "heap_int.h"
#include "heap_st.h"
#include "velum/klog.h"

static const t_st_case	g_cases[] = {{"tailles", st_sizes},
{"alignements", st_aligned}, {"etiquettes", st_tags},
{"realloc et calloc", st_realloc}, {"echecs injectes", st_failures},
{"charge aleatoire", st_stress}, {NULL, NULL}};

uint64_t	st_live(const t_heap_stats *s)
{
	uint64_t	sum;
	uint32_t	t;

	sum = 0;
	t = 0;
	while (t < HEAP_TAGS)
	{
		sum += s->allocs_live[t];
		t++;
	}
	return (sum);
}

uint64_t	st_bytes(const t_heap_stats *s)
{
	uint64_t	sum;
	uint32_t	t;

	sum = 0;
	t = 0;
	while (t < HEAP_TAGS)
	{
		sum += s->bytes_live[t];
		t++;
	}
	return (sum);
}

static int	selftest_balance(const t_heap_stats *before)
{
	t_heap_stats	after;
	int				fails;

	heap_trim();
	heap_get_stats(&after);
	fails = (st_live(before) != st_live(&after));
	fails += (heap_check() != 0);
	klog_info("heap: au repos %llu objets, %llu octets, %llu pages",
		(unsigned long long)st_live(&after),
		(unsigned long long)st_bytes(&after),
		(unsigned long long)after.pages_mapped);
	return (fails);
}

int	heap_selftest(void)
{
	t_heap_stats	before;
	int				fails;
	int				rc;
	uint32_t		i;

	heap_get_stats(&before);
	fails = 0;
	i = 0;
	while (g_cases[i].fn)
	{
		rc = g_cases[i].fn();
		if (rc)
			klog_err("heap: essai '%s' en echec (%d)", g_cases[i].name, rc);
		fails += (rc != 0);
		i++;
	}
	st_bench();
	return (fails + selftest_balance(&before));
}
