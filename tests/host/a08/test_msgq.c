#include "harness.h"
#include "fake.h"

static void	drain(t_msgq *q)
{
	t_ipcmsg	*m;

	m = msgq_pop(q);
	while (m)
	{
		msg_free(m);
		m = msgq_pop(q);
	}
}

static void	fifo_order(void)
{
	t_msgq		q;
	t_ipcmsg	*m;

	fk_reset();
	q = (t_msgq){NULL, NULL, 0, 0};
	msgq_push(&q, msg_alloc(1), IPC_QUEUE_MAX);
	msgq_push(&q, msg_alloc(2), IPC_QUEUE_MAX);
	msgq_push(&q, msg_alloc(3), IPC_QUEUE_MAX);
	m = msgq_pop(&q);
	h_eq_i64("premier", m->len, 1);
	msgq_push_front(&q, m);
	h_eq_i64("remis en tete", q.head->len, 1);
	h_eq_i64("compte", q.count, 3);
	msg_free(msgq_pop(&q));
	m = msgq_pop(&q);
	h_eq_i64("deuxieme", m->len, 2);
	msg_free(m);
	m = msgq_pop(&q);
	h_eq_i64("troisieme", m->len, 3);
	h_true(q.head == NULL && q.tail == NULL && q.count == 0, "file vide");
	msg_free(m);
	h_true(msgq_pop(&q) == NULL, "pop sur vide");
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	full_bound(void)
{
	t_msgq		q;
	t_ipcmsg	*m;
	int			i;
	int			ok;

	fk_reset();
	q = (t_msgq){NULL, NULL, 0, 0};
	ok = 0;
	i = 0;
	while (i++ < IPC_QUEUE_MAX)
		ok += (msgq_push(&q, msg_alloc(0), IPC_QUEUE_MAX) == 0);
	h_eq_i64("64 messages", ok, IPC_QUEUE_MAX);
	m = msg_alloc(0);
	h_eq_i64("file pleine", msgq_push(&q, m, IPC_QUEUE_MAX), E_AGAIN);
	h_eq_i64("compte inchange", q.count, IPC_QUEUE_MAX);
	msgq_push_front(&q, m);
	h_eq_i64("remise en tete hors borne", q.count, IPC_QUEUE_MAX + 1);
	drain(&q);
	m = msg_alloc(0);
	msgq_push_front(&q, m);
	h_true(q.head == m && q.tail == m, "remise en tete sur vide");
	drain(&q);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	alloc_bounds(void)
{
	t_ipcmsg	*m;
	t_object	*e;

	fk_reset();
	m = msg_alloc(IPC_MSG_MAX);
	h_true(m && m->len == IPC_MSG_MAX && m->nh == 0, "taille max");
	msg_free(m);
	h_true(msg_alloc(IPC_MSG_MAX + 1) == NULL, "taille max + 1");
	g_fk.heap_fail = 1;
	h_true(msg_alloc(0) == NULL, "tas plein");
	evt_create(0, 0, &e);
	m = msg_alloc(0);
	obj_ref(e);
	m->objs[0] = e;
	m->nh = 1;
	msg_free(m);
	h_eq_i64("objet transporte rendu", e->refs, 1);
	msg_free(NULL);
	obj_unref(e);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/msgq");
	h_run("fifo_order", fifo_order);
	h_run("full_bound", full_bound);
	h_run("alloc_bounds", alloc_bounds);
	return (h_end());
}
