#include "harness.h"
#include "fake.h"

static char	g_buf[128];

static void	send_charged(void)
{
	t_process	*p;
	t_handle	c[2];
	t_chansend	cs;
	uint64_t	cost;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	cost = sizeof(t_ipcmsg) + 100;
	p->mem_limit = 2 * cost;
	fk_chan_pair(p, p, c);
	cs = (t_chansend){fk_uptr(g_buf), 100, 0, 0, 0};
	h_eq_i64("premier message", fk_send(c[0], &cs), 0);
	h_eq_i64("second message", fk_send(c[0], &cs), 0);
	h_eq_i64("plafond atteint", fk_send(c[0], &cs), E_NOMEM);
	h_eq_i64("compte", p->handles->acct->bytes, 2 * cost);
	h_eq_i64("lecture", fk_recv(c[1], &(t_chanrecv){fk_uptr(g_buf), 128, 0,
			0, 0, 0, 0, 0, 0}), 0);
	h_eq_i64("rendu a la lecture", p->handles->acct->bytes, cost);
	fk_call(SYS_CLOSE, c[1], 0, 0);
	h_eq_i64("rendu a la fermeture", p->handles->acct->bytes, 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	sender_dies_first(void)
{
	t_process	*pa;
	t_process	*pb;
	t_handle	c[2];

	fk_reset();
	pa = fk_proc_new(0);
	pb = fk_proc_new(0);
	fk_chan_pair(pa, pb, c);
	g_fk.cur = pa;
	fk_send(c[0], &(t_chansend){fk_uptr(g_buf), 8, 0, 0, 0});
	fk_proc_free(pa);
	g_fk.cur = pb;
	h_eq_i64("message lisible apres la mort", fk_recv(c[1], &(t_chanrecv){
			fk_uptr(g_buf), 128, 0, 0, 0, 0, 0, 0, 0}), 0);
	h_eq_i64("puis tuyau casse", fk_recv(c[1], &(t_chanrecv){0, 0, 0, 0, 0,
			0, 0, 0, 0}), E_PIPE);
	fk_proc_free(pb);
	h_eq_i64("compte et tas rendus", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/ipcmem");
	h_run("send_charged", send_charged);
	h_run("sender_dies_first", sender_dies_first);
	return (h_end());
}
