#include "harness.h"
#include "fake.h"

static void	write_read_overflow(void)
{
	t_object	*a;
	t_object	*b;
	t_chanrd	rd;

	fk_reset();
	h_eq_i64("paire", chan_create(&a, &b), 0);
	h_true(!chan_signaled(b), "vide : non signale");
	h_eq_i64("ecriture", chan_write(a, msg_alloc(8)), 0);
	h_true(chan_signaled(b) && !chan_signaled(a), "signal du bon cote");
	rd = (t_chanrd){NULL, 4, 0, 0, 0};
	h_eq_i64("tampon court", chan_read_try(b, &rd), E_OVERFLOW);
	h_eq_i64("taille demandee", rd.need_len, 8);
	h_true(rd.msg == NULL && chan_signaled(b), "message garde");
	rd.buf_len = 8;
	h_eq_i64("lecture", chan_read_try(b, &rd), 0);
	h_eq_i64("longueur", rd.msg->len, 8);
	msg_free(rd.msg);
	h_eq_i64("vide", chan_read_try(b, &rd), E_AGAIN);
	h_eq_i64("delai", chan_read_wait(b, &rd, 0), E_TIMEOUT);
	h_true(chan_same_pair(a, b) && !chan_same_pair(a, NULL), "meme paire");
	obj_unref(a);
	obj_unref(b);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	close_pipe(void)
{
	t_object	*a;
	t_object	*b;
	t_chanrd	rd;
	t_ipcmsg	*m;

	fk_reset();
	chan_create(&a, &b);
	chan_write(a, msg_alloc(1));
	chan_write(a, msg_alloc(2));
	obj_unref(a);
	rd = (t_chanrd){NULL, 16, 0, 0, 0};
	h_eq_i64("premier apres fermeture", chan_read_try(b, &rd), 0);
	msg_free(rd.msg);
	h_eq_i64("second", chan_read_wait(b, &rd, TIMEOUT_INF), 0);
	msg_free(rd.msg);
	h_eq_i64("tuyau casse", chan_read_wait(b, &rd, TIMEOUT_INF), E_PIPE);
	h_true(chan_signaled(b), "signale quand l'autre bout est ferme");
	m = msg_alloc(0);
	h_eq_i64("ecriture vers ferme", chan_write(b, m), E_PIPE);
	msg_free(m);
	obj_unref(b);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	full_queue(void)
{
	t_object	*a;
	t_object	*b;
	t_object	*x;
	t_ipcmsg	*m;
	int			i;

	fk_reset();
	chan_create(&a, &b);
	i = 0;
	while (i < IPC_QUEUE_MAX && chan_write(a, msg_alloc(0)) == 0)
		i++;
	h_eq_i64("64 ecritures", i, IPC_QUEUE_MAX);
	m = msg_alloc(0);
	h_eq_i64("file pleine", chan_write(a, m), E_AGAIN);
	msg_free(m);
	h_eq_i64("l'autre sens est libre", chan_write(b, msg_alloc(0)), 0);
	g_fk.heap_fail = 1;
	h_eq_i64("paire sans memoire", chan_create(&x, &x), E_NOMEM);
	g_fk.heap_fail = 2;
	h_eq_i64("bout sans memoire", chan_create(&x, &x), E_NOMEM);
	obj_unref(a);
	obj_unref(b);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/chan");
	h_run("write_read_overflow", write_read_overflow);
	h_run("close_pipe", close_pipe);
	h_run("full_queue", full_queue);
	return (h_end());
}
