#include "rng_int.h"
#include "velum/boot.h"
#include "velum/cpu.h"
#include "velum/heap.h"
#include "velum/libk.h"
#include "velum/pmm.h"
#include "velum/proc.h"
#include "velum/timer.h"
#include "velum/util.h"

void	sysinfo_build(t_sysinfo *out, const t_sysinfo_in *in)
{
	memset(out, 0, sizeof(*out));
	out->abi_version = VELUM_ABI_VERSION;
	out->ncpus = in->ncpus;
	out->nprocs = in->nprocs;
	out->uptime_ns = in->uptime_ns;
	out->mem_total = in->mem_total;
	out->mem_free = in->mem_free;
	out->heap_live = in->heap_live;
	if (in->cpu_brand)
		strlcpy(out->cpu_brand, in->cpu_brand, sizeof(out->cpu_brand));
	if (in->build_id)
		strlcpy(out->build_id, in->build_id, sizeof(out->build_id));
	strlcpy(out->os_name, "VelumOS", sizeof(out->os_name));
}

static uint32_t	count_procs(void)
{
	int	n;

	n = proc_count();
	if (n < 0)
		return (0);
	return ((uint32_t)n);
}

static uint64_t	heap_bytes_live(void)
{
	t_heap_stats	st;
	uint64_t		sum;
	uint32_t		i;

	heap_get_stats(&st);
	sum = 0;
	i = 0;
	while (i < HEAP_TAGS)
	{
		sum += st.bytes_live[i];
		i++;
	}
	return (sum);
}

void	sysinfo_collect(t_sysinfo *out)
{
	t_sysinfo_in	in;
	t_pmm_stats		ps;
	const t_cpufeat	*f;

	pmm_get_stats(&ps);
	f = cpu_features();
	in.ncpus = cpu_count();
	in.nprocs = count_procs();
	in.uptime_ns = time_now_ns();
	in.mem_total = ps.total_pages * PAGE_SIZE;
	in.mem_free = ps.free_pages * PAGE_SIZE;
	in.heap_live = heap_bytes_live();
	in.cpu_brand = NULL;
	if (f)
		in.cpu_brand = f->brand;
	in.build_id = boot_build_id();
	sysinfo_build(out, &in);
}
