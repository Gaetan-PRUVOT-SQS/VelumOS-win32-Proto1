#include "proc_int.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/util.h"

static int	tstack_size(uint64_t req, uint64_t *out)
{
	if (req == 0)
		req = TSTACK_DEFAULT;
	if (req < TSTACK_MIN || req > TSTACK_MAX)
		return (E_INVAL);
	*out = align_up(req, PAGE_SIZE);
	return (0);
}

int	proc_tstack_alloc(t_process *p, uint64_t size, uint64_t arg,
		t_pthread *st)
{
	uint64_t	frame[2];
	uintptr_t	va;
	uint64_t	len;
	int			rc;

	va = 0;
	rc = tstack_size(size, &len);
	if (rc == 0)
		rc = vmm_stack_map(p->aspace, &va, len);
	if (rc < 0)
		return (rc);
	st->stack_va = va;
	st->stack_len = len + PAGE_SIZE;
	frame[0] = arg;
	frame[1] = 0;
	rc = aspace_write(p->aspace, st->stack_va + st->stack_len - sizeof(frame),
			frame, sizeof(frame));
	if (rc < 0 && vmm_stack_unmap(p->aspace, va, len) < 0)
		klog_warn("proc: pile de fil %#llx non rendue",
			(unsigned long long)va);
	return (rc);
}
