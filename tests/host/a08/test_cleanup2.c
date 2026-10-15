#include <stdlib.h>
#include "harness.h"
#include "fake.h"

static void	wait_many_bounds(uint64_t list)
{
	h_eq_i64("n = 0", fk_call(SYS_WAIT_MANY, list, 0, 0), E_INVAL);
	h_eq_i64("n = 65", fk_call(SYS_WAIT_MANY, list, 65, 0), E_INVAL);
	h_eq_i64("drapeau", fk_call(SYS_WAIT_MANY, list, 1, 2), E_INVAL);
	h_eq_i64("liste nulle", fk_call(SYS_WAIT_MANY, 0, 1, 0), E_FAULT);
}

static void	wait_many_syscall(void)
{
	t_process	*p;
	t_handle	hs[3];
	uint64_t	a[6];

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	hs[0] = fk_event_in(p, HR_ALL);
	hs[1] = fk_event_in(p, HR_ALL);
	hs[2] = fk_event_in(p, HR_SIGNAL);
	a[0] = fk_uptr(hs);
	a[1] = 2;
	a[2] = WAIT_ANY;
	a[3] = 0;
	fk_call(SYS_EVENT_OP, hs[1], EV_SET, 0);
	h_eq_i64("indice", fk_call6(SYS_WAIT_MANY, a), 1);
	a[2] = WAIT_ALL;
	h_eq_i64("tous : delai", fk_call6(SYS_WAIT_MANY, a), E_TIMEOUT);
	a[1] = 3;
	h_eq_i64("sans HR_WAIT", fk_call6(SYS_WAIT_MANY, a), E_PERM);
	h_eq_i64("references rendues", fk_obj(p, hs[0])->refs, 1);
	wait_many_bounds(a[0]);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	destroy_without_cleanup(void)
{
	t_process	*p;
	t_object	*s;
	t_secreq	rq;
	uintptr_t	va;

	fk_reset();
	p = fk_proc_new(0);
	section_create(p, PAGE_SIZE, PROT_R, &s);
	rq = (t_secreq){0, 0, PAGE_SIZE, PROT_R, 0};
	h_eq_i64("mappage", section_map(p, s, &rq, &va), 0);
	handle_table_destroy(p->handles);
	p->handles = NULL;
	h_eq_i64("section gardee plutot que liberee mappee", s->refs, 2);
	h_eq_i64("frame toujours vivante", g_fk.pmm_live, 1);
	vmm_unmap(p->aspace, va, PAGE_SIZE);
	obj_unref(s);
	obj_unref(s);
	free(p->aspace);
	free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/cleanup2");
	h_run("wait_many_syscall", wait_many_syscall);
	h_run("destroy_without_cleanup", destroy_without_cleanup);
	return (h_end());
}
