#include "heap_int.h"
#include "heap_st.h"
#include "velum/klog.h"

static const uint32_t	g_bench[] = {32, 256, 2048, 8192};

static uint64_t	bench_pair(size_t size, uint32_t loops)
{
	uint64_t	start;
	uint32_t	i;

	kfree(kmalloc(size));
	start = __builtin_ia32_rdtsc();
	i = 0;
	while (i < loops)
	{
		kfree(kmalloc(size));
		i++;
	}
	return ((__builtin_ia32_rdtsc() - start) / loops);
}

static uint64_t	bench_burst(size_t size)
{
	void		*p[ST_BURST];
	uint64_t	start;
	uint32_t	i;

	start = __builtin_ia32_rdtsc();
	i = 0;
	while (i < ST_BURST)
	{
		p[i] = kmalloc(size);
		i++;
	}
	while (i > 0)
	{
		i--;
		kfree(p[i]);
	}
	return ((__builtin_ia32_rdtsc() - start) / (2 * ST_BURST));
}

void	st_bench(void)
{
	uint32_t	i;
	uint32_t	loops;

	i = 0;
	while (i < sizeof(g_bench) / sizeof(g_bench[0]))
	{
		loops = ST_LOOPS;
		if (g_bench[i] > HEAP_SMALL_MAX)
			loops = ST_LOOPS_LARGE;
		klog_info("heap: mesure %u octets : %llu cycles par paire, "
			"%llu cycles par operation en rafale", g_bench[i],
			(unsigned long long)bench_pair(g_bench[i], loops),
			(unsigned long long)bench_burst(g_bench[i]));
		i++;
	}
}
