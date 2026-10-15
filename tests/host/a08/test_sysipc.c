#include "harness.h"
#include "fake.h"

static char	g_bad[64];

static void	send_fields(t_handle *c, t_handle ev)
{
	t_chansend	cs;
	t_handle	hs[1];

	hs[0] = ev;
	cs = (t_chansend){fk_uptr("x"), 1, fk_uptr(hs), 1, 2};
	h_eq_i64("drapeau inconnu", fk_send(c[0], &cs), E_INVAL);
	cs = (t_chansend){fk_uptr("x"), IPC_MSG_MAX + 1, 0, 0, 0};
	h_eq_i64("message trop long", fk_send(c[0], &cs), E_INVAL);
	cs = (t_chansend){0, 0, fk_uptr(hs), IPC_HANDLES_MAX + 1, 0};
	h_eq_i64("trop de handles", fk_send(c[0], &cs), E_INVAL);
	h_eq_i64("structure nulle", fk_call(SYS_CHAN_SEND, c[0], 0, 0), E_FAULT);
	cs = (t_chansend){fk_uptr(g_bad), 1, 0, 0, 0};
	h_eq_i64("message illisible", fk_send(c[0], &cs), E_FAULT);
	cs = (t_chansend){0, 0, fk_uptr(g_bad), 1, 0};
	h_eq_i64("liste illisible", fk_send(c[0], &cs), E_FAULT);
	cs = (t_chansend){0, 0, 0, 0, 0};
	h_eq_i64("handle d'evenement", fk_send(ev, &cs), E_BADF);
	h_eq_i64("handle 0", fk_send(0, &cs), E_BADF);
	h_eq_i64("handle 64 bits", fk_call(SYS_CHAN_SEND, 1ull << 32,
			fk_uptr(&cs), 0), E_BADF);
	h_eq_i64("sans HR_WRITE", fk_send(c[2], &cs), E_PERM);
	h_eq_i64("message vide accepte", fk_send(c[0], &cs), 0);
}

static void	recv_fields(t_handle *c)
{
	t_chanrecv	cr;

	cr = (t_chanrecv){0, 0, 0, 0, 1, 0, 0, 0, 0};
	h_eq_i64("reserve", fk_recv(c[1], &cr), E_INVAL);
	cr = (t_chanrecv){0, 0, 0, 0, 0, 0, 0, 0, 1};
	h_eq_i64("reserve2", fk_recv(c[1], &cr), E_INVAL);
	h_eq_i64("structure nulle", fk_call(SYS_CHAN_RECV, c[1], 0, 0), E_FAULT);
	cr = (t_chanrecv){0, 0, 0, 0, 0, 0, 0, 0, 0};
	h_eq_i64("sans HR_READ", fk_recv(c[3], &cr), E_PERM);
	h_eq_i64("message vide lu", fk_recv(c[1], &cr), 0);
	h_eq_i64("vide : delai", fk_recv(c[1], &cr), E_TIMEOUT);
}

static void	validation(void)
{
	t_process	*p;
	t_handle	c[4];
	t_handle	ev;

	fk_reset();
	g_fk.fault_lo = fk_uptr(g_bad);
	g_fk.fault_hi = fk_uptr(g_bad) + sizeof(g_bad);
	p = fk_proc_new(0);
	g_fk.cur = p;
	fk_chan_pair(p, p, c);
	handle_dup_as(p, c[0], HR_READ, &c[2]);
	handle_dup_as(p, c[1], HR_WRITE, &c[3]);
	ev = fk_event_in(p, RIGHTS_SIGOBJ);
	send_fields(c, ev);
	recv_fields(c);
	handle_close(p, c[0]);
	handle_close(p, c[2]);
	h_eq_i64("bout ferme", fk_recv(c[1], &(t_chanrecv){0, 0, 0, 0, 0,
			0, 0, 0, 0}), E_PIPE);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	recv_failures(void)
{
	t_process	*p;
	t_handle	c[2];
	t_handle	hs[2];
	t_chanrecv	cr;
	char		buf[8];

	fk_reset();
	g_fk.fault_lo = fk_uptr(g_bad);
	g_fk.fault_hi = fk_uptr(g_bad) + sizeof(g_bad);
	p = fk_proc_new(0);
	g_fk.cur = p;
	fk_chan_pair(p, p, c);
	hs[0] = fk_event_in(p, RIGHTS_SIGOBJ);
	fk_send(c[0], &(t_chansend){fk_uptr("abcd"), 4, fk_uptr(hs), 1, 0});
	cr = (t_chanrecv){fk_uptr(buf), 2, fk_uptr(hs), 8, 0, 0, 0, 0, 0};
	h_eq_i64("tampon court", fk_recv(c[1], &cr), E_OVERFLOW);
	h_true(cr.out_len == 4 && cr.out_nhandles == 1, "tailles demandees");
	cr = (t_chanrecv){fk_uptr(g_bad), 8, fk_uptr(hs), 8, 0, 0, 0, 0, 0};
	h_eq_i64("tampon illisible", fk_recv(c[1], &cr), E_FAULT);
	h_eq_i64("aucune reservation perdue", p->handles->used, 3);
	cr = (t_chanrecv){fk_uptr(buf), 8, fk_uptr(hs), 8, 0, 0, 0, 0, 0};
	h_eq_i64("message toujours la", fk_recv(c[1], &cr), 0);
	h_eq_i64("handle recu", p->handles->used, 4);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/sysipc");
	h_run("validation", validation);
	h_run("recv_failures", recv_failures);
	return (h_end());
}
