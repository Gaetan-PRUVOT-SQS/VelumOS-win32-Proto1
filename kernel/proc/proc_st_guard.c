#include "proc_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/klog.h"
#include "velum/pmm.h"
#include "velum/util.h"

#define STG_TOP 0x40000000ull
#define STG_SPAN 0x101000ull
#define STG_SWEEP 8
#define STG_THREADS 1000

static int	stg_present(t_aspace *as, uint64_t guard)
{
	t_vminfo	vi;

	if (vmm_query(as, guard, &vi))
		return (1);
	if (vmm_find_free(as, PAGE_SIZE, guard, guard + PAGE_SIZE) != 0)
		return (2);
	if (vmm_alloc(as, guard, PAGE_SIZE, VM_USER | VM_R | VM_W) != E_EXIST)
		return (3);
	if (!vmm_query(as, guard + PAGE_SIZE, &vi) || !(vi.flags & VM_USER))
		return (4);
	return (0);
}

static int	stg_fail_at(t_aspace *as, int64_t n)
{
	uint64_t	irq;
	int			rc;

	irq = irq_save();
	pmm_fail_after(n);
	rc = proc_stack_map(as, STG_TOP);
	pmm_fail_after(-1);
	irq_restore(irq);
	if (rc != E_NOMEM)
		return (5);
	if (vmm_find_free(as, STG_SPAN, STG_TOP - STG_SPAN, STG_TOP)
		!= STG_TOP - STG_SPAN)
		return (6);
	return (0);
}

static int	stg_threads(t_aspace *as)
{
	t_process	*p;
	t_pthread	st;
	t_vminfo	vi;
	uint64_t	start;
	uint32_t	i;

	p = kcalloc(1, sizeof(*p));
	if (!p)
		return (9);
	p->aspace = as;
	start = vmm_region_count(as);
	i = 0;
	while (i < STG_THREADS && proc_tstack_alloc(p, TSTACK_MIN, i, &st) == 0
		&& vmm_region_count(as) == start + 2
		&& !vmm_query(as, st.stack_va, &vi)
		&& vmm_stack_unmap(as, st.stack_va, st.stack_len - PAGE_SIZE) == 0)
		i++;
	kfree(p);
	if (i < STG_THREADS || vmm_region_count(as) != start)
		return (10);
	return (0);
}

static int	stg_run(t_aspace *as)
{
	int64_t	n;
	int		rc;

	rc = stg_threads(as);
	n = 0;
	while (rc == 0 && n < STG_SWEEP)
		rc = stg_fail_at(as, n++);
	if (rc == 0 && proc_stack_map(as, STG_TOP) < 0)
		rc = 7;
	if (rc == 0)
		rc = stg_present(as, STG_TOP - STG_SPAN);
	if (rc == 0 && proc_stack_map(as, USER_MIN) != E_INVAL)
		rc = 8;
	return (rc);
}

int	proc_selftest(void)
{
	t_aspace	*as;
	int			rc;

	if (proc_spawn_selftest())
		return (1);
	if (proc_count() < 0 || proc_count() > PROC_MAX)
		return (1);
	as = vmm_aspace_create();
	if (!as)
		return (1);
	rc = stg_run(as);
	vmm_aspace_destroy(as);
	if (rc)
		klog_err("proc: autotest de la garde de pile, étape %d", rc);
	else
		klog_info("proc: garde de pile réservée, échecs d'allocation ok");
	return (rc != 0);
}
