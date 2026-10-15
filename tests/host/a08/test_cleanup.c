#include <stdlib.h>
#include "harness.h"
#include "fake.h"

static void	cleanup_setup(t_process *p, t_process *q, t_handle *c)
{
	int64_t		s;

	p->obj = obj_process_new(p);
	fk_chan_pair(p, q, c);
	g_fk.cur = p;
	fk_event_in(p, HR_ALL);
	fk_call(SYS_PORT_LISTEN, fk_uptr("svc"), 3, 0);
	s = fk_call(SYS_SECTION_CREATE, 2 * PAGE_SIZE, PROT_R, 0);
	fk_map(s, 0, PROT_R, 0);
}

static void	process_cleanup(void)
{
	t_process	*p;
	t_process	*q;
	t_handle	c[2];

	fk_reset();
	p = fk_proc_new(PF_LISTEN);
	q = fk_proc_new(0);
	cleanup_setup(p, q, c);
	h_eq_i64("deux pages mappees", fk_count_maps(NULL), 2);
	object_process_cleanup(p);
	h_eq_i64("handles fermes", p->handles->used, 0);
	h_eq_i64("pages demappees", fk_count_maps(NULL), 0);
	h_eq_i64("frames rendues", g_fk.pmm_live, 0);
	h_true(p->obj->ops->signaled(p->obj), "objet processus signale");
	g_fk.cur = q;
	h_eq_i64("pair : tuyau casse", fk_recv(c[1], &(t_chanrecv){0, 0, 0, 0, 0,
			0, 0, 0, 0}), E_PIPE);
	h_eq_i64("nom libere", fk_call(SYS_PORT_CONNECT, fk_uptr("svc"), 3, 0),
		E_NOENT);
	obj_unref(p->obj);
	p->obj = NULL;
	h_eq_i64("reference processus rendue", g_fk.proc_refs, 0);
	fk_proc_free(p);
	fk_proc_free(q);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	task_objects(void)
{
	t_process	*p;
	t_thread	*t;
	t_object	*o;

	fk_reset();
	p = fk_proc_new(0);
	t = calloc(1, sizeof(t_thread));
	h_true(!obj_process_new(NULL) && !obj_thread_new(NULL), "NULL refuse");
	o = obj_process_new(p);
	h_true(!o->ops->signaled(o) && obj_process_of(o) == p, "vivant");
	p->state = PS_ZOMBIE;
	h_true(o->ops->signaled(o) && !obj_thread_of(o), "zombie signale");
	obj_unref(o);
	o = obj_thread_new(t);
	h_true(obj_thread_of(o) == t && g_fk.thread_refs == 1, "fil");
	obj_task_exited(o);
	h_true(o->ops->signaled(o) && !obj_process_of(o), "fil sorti signale");
	obj_unref(o);
	h_true(g_fk.thread_refs == 0 && g_fk.proc_refs == 0, "refs rendues");
	g_fk.heap_fail = 2;
	h_true(!obj_thread_new(t), "objet sans memoire");
	obj_task_exited(NULL);
	free(t);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/cleanup");
	h_run("process_cleanup", process_cleanup);
	h_run("task_objects", task_objects);
	return (h_end());
}
