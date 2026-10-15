#include "proc_int.h"
#include "velum/err.h"
#include "velum/libk.h"

uint64_t	proc_random_slot(uint64_t n)
{
	if (n == 0 || !krandom_below)
		return (0);
	return (krandom_below(n));
}

static uint64_t	stack_pick_top(const t_spawnctx *c)
{
	uint64_t	top;
	uint64_t	slots;
	uint32_t	tries;

	if (!krandom_below)
		return (USER_STACK_TOP);
	slots = (ELF_ASLR_HI - ELF_ASLR_LO) / USTACK_SLOT - 1;
	tries = 0;
	while (tries < 8)
	{
		top = ELF_ASLR_LO + (proc_random_slot(slots) + 1) * USTACK_SLOT;
		if (top <= c->base + c->info.span_lo
			|| top - USTACK_SLOT >= c->base + c->info.span_hi)
			return (top);
		tries++;
	}
	return (USER_STACK_TOP);
}

static void	stack_random(uint8_t *rnd)
{
	uint64_t	v;

	if (krandom)
	{
		krandom(rnd, USTACK_RANDOM);
		return ;
	}
	v = __builtin_ia32_rdtsc() * 0x9e3779b97f4a7c15ull;
	memcpy(rnd, &v, sizeof(v));
	v ^= v >> 29;
	v *= 0xbf58476d1ce4e5b9ull;
	memcpy(rnd + sizeof(v), &v, sizeof(v));
}

static void	stack_start_info(const t_spawnctx *c, t_ustart *in,
				const uint8_t *rnd)
{
	memset(in, 0, sizeof(*in));
	in->path = c->path;
	in->path_len = c->path_len;
	in->args = c->rq->args;
	in->args_len = c->rq->args_len;
	in->nargs = (uint32_t)c->nargs;
	in->flags = c->p->flags;
	in->rnd = rnd;
	in->entry = c->p->entry;
	if (c->info.phdr_vaddr)
		in->phdr = c->base + c->info.phdr_vaddr;
	in->phnum = c->info.phnum;
}

int	proc_setup_stack(t_spawnctx *c)
{
	t_ustack	st;
	t_ustart	in;
	uint8_t		rnd[USTACK_RANDOM];
	int			rc;

	st.top = stack_pick_top(c);
	rc = proc_stack_map(c->p->aspace, st.top);
	if (rc < 0)
		return (rc);
	st.cap = USTACK_INIT_MAX;
	st.buf = kmalloc_tag(st.cap, HEAP_PROC);
	if (!st.buf)
		return (E_NOMEM);
	stack_random(rnd);
	stack_start_info(c, &in, rnd);
	rc = ustack_build(&st, &in);
	if (rc == 0)
		rc = aspace_write(c->p->aspace, st.sp,
				st.buf + st.cap - (st.top - st.sp), st.top - st.sp);
	c->sp = st.sp;
	memset(st.buf, 0, st.cap);
	kfree(st.buf);
	memset(rnd, 0, sizeof(rnd));
	return (rc);
}
