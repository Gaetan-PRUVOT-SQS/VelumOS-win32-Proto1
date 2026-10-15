#include <string.h>
#include "harness.h"
#include "fake.h"

static void	move_success(void)
{
	t_process	*pa;
	t_process	*pb;
	t_handle	c[2];
	t_handle	hs[IPC_HANDLES_MAX];
	char		buf[16];

	fk_reset();
	pa = fk_proc_new(0);
	pb = fk_proc_new(0);
	fk_chan_pair(pa, pb, c);
	hs[0] = fk_event_in(pa, RIGHTS_SIGOBJ);
	g_fk.cur = pa;
	h_eq_i64("envoi avec deplacement", fk_send(c[0], &(t_chansend){fk_uptr(
				"abc"), 3, fk_uptr(hs), 1, CHAN_SEND_MOVE}), 0);
	h_true(fk_obj(pa, hs[0]) == NULL, "handle sorti de l'emetteur");
	g_fk.cur = pb;
	h_eq_i64("reception", fk_recv(c[1], &(t_chanrecv){fk_uptr(buf), 16,
			fk_uptr(hs), 8, 0, 0, 0, 0, 0}), 0);
	h_true(memcmp(buf, "abc", 3) == 0, "donnees recues");
	h_true(fk_obj(pb, hs[0]) && fk_obj(pb, hs[0])->refs == 1,
		"handle installe, seul referent");
	fk_proc_free(pa);
	fk_proc_free(pb);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	dup_success(void)
{
	t_process	*pa;
	t_process	*pb;
	t_handle	c[2];
	t_handle	hs[2];
	t_chanrecv	cr;

	fk_reset();
	pa = fk_proc_new(0);
	pb = fk_proc_new(0);
	fk_chan_pair(pa, pb, c);
	hs[0] = fk_event_in(pa, RIGHTS_SIGOBJ & ~HR_SIGNAL);
	g_fk.cur = pa;
	h_eq_i64("envoi par copie", fk_send(c[0], &(t_chansend){0, 0,
			fk_uptr(hs), 1, 0}), 0);
	h_true(fk_obj(pa, hs[0]) != NULL, "handle garde par l'emetteur");
	g_fk.cur = pb;
	cr = (t_chanrecv){0, 0, fk_uptr(&hs[1]), 1, 0, 0, 7, 7, 0};
	h_eq_i64("reception", fk_recv(c[1], &cr), 0);
	h_true(cr.out_len == 0 && cr.out_nhandles == 1, "tailles ecrites");
	h_true(fk_obj(pb, hs[1]) == fk_obj(pa, hs[0]), "meme objet");
	h_eq_i64("droits", pb->handles->ents[hs[1] & HT_IDX_MASK].rights, 0x181);
	fk_proc_free(pa);
	fk_proc_free(pb);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	refused_sends(t_process *pa, t_handle *c, t_handle *hs)
{
	t_chansend	cs;

	cs = (t_chansend){0, 0, fk_uptr(hs), 2, CHAN_SEND_MOVE};
	hs[1] = hs[0];
	h_eq_i64("handle en double", fk_send(c[0], &cs), E_INVAL);
	hs[1] = fk_event_in(pa, HR_WAIT);
	h_eq_i64("sans HR_TRANSFER", fk_send(c[0], &cs), E_PERM);
	hs[1] = c[0];
	h_eq_i64("bout lui-meme", fk_send(c[0], &cs), E_INVAL);
	hs[1] = c[2];
	h_eq_i64("bout d'en face", fk_send(c[0], &cs), E_INVAL);
	hs[1] = 12345;
	h_eq_i64("handle invalide", fk_send(c[0], &cs), E_BADF);
	cs.nhandles = 0;
	while (fk_send(c[0], &cs) == 0)
		;
	cs.nhandles = 1;
	h_eq_i64("file pleine", fk_send(c[0], &cs), E_AGAIN);
}

static void	atomic_failures(void)
{
	t_process	*pa;
	t_process	*pb;
	t_handle	c[3];
	t_handle	hs[2];

	fk_reset();
	pa = fk_proc_new(0);
	pb = fk_proc_new(0);
	fk_chan_pair(pa, pb, c);
	handle_dup(pb, c[1], pa, &c[2]);
	hs[0] = fk_event_in(pa, RIGHTS_SIGOBJ);
	g_fk.cur = pa;
	refused_sends(pa, c, hs);
	h_true(fk_obj(pa, hs[0]) != NULL, "handle intact apres echecs");
	h_eq_i64("une seule reference", fk_obj(pa, hs[0])->refs, 1);
	h_eq_i64("entree utilisable", pa->handles->ents[hs[0]
		& HT_IDX_MASK].state, HE_USED);
	fk_proc_free(pa);
	fk_proc_free(pb);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/xfer");
	h_run("move_success", move_success);
	h_run("dup_success", dup_success);
	h_run("atomic_failures", atomic_failures);
	return (h_end());
}
