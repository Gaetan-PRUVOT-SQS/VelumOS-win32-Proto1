#include <string.h>
#include "harness.h"
#include "fake.h"

static const char	*g_names[] = {"", "a", "A", "a/b", "winsrv.main-1_x",
	"a\0b", " ", "\xc3\xa9", "0123456789abcdef0123456789abcdef0123456789"
	"abcdef0123456789abcdef", "0123456789abcdef0123456789abcdef0123456789"
	"abcdef0123456789abcdefg"};
static const int	g_ok[] = {0, 1, 0, 0, 1, 0, 0, 0, 1, 0};

static void	name_grammar(void)
{
	int		i;
	int		bad;

	bad = 0;
	i = 0;
	while (i < 10)
	{
		bad += (port_name_ok(g_names[i], strlen(g_names[i]) + (i == 5) * 2)
				!= g_ok[i]);
		i++;
	}
	h_eq_i64("table des noms", bad, 0);
	h_true(port_name_ok(g_names[8], 64), "64 octets acceptes");
	h_true(!port_name_ok(g_names[9], 65), "65 octets refuses");
	h_true(!port_name_ok(NULL, 1), "nom NULL");
}

static void	listen_rules(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h_eq_i64("sans PF_LISTEN", fk_call(SYS_PORT_LISTEN, fk_uptr("svc"), 3, 0),
		E_PERM);
	p->flags = PF_LISTEN;
	h = fk_call(SYS_PORT_LISTEN, fk_uptr("svc"), 3, 0);
	h_true(h > 0, "ecoute");
	h_eq_i64("nom pris", fk_call(SYS_PORT_LISTEN, fk_uptr("svc"), 3, 0),
		E_EXIST);
	h_eq_i64("drapeau", fk_call(SYS_PORT_LISTEN, fk_uptr("x"), 1, 1), E_INVAL);
	h_eq_i64("longueur 0", fk_call(SYS_PORT_LISTEN, fk_uptr("x"), 0, 0),
		E_INVAL);
	h_eq_i64("longueur 65", fk_call(SYS_PORT_LISTEN, fk_uptr("x"), 65, 0),
		E_INVAL);
	h_eq_i64("pointeur nul", fk_call(SYS_PORT_LISTEN, 0, 3, 0), E_FAULT);
	h_eq_i64("nom invalide", fk_call(SYS_PORT_LISTEN, fk_uptr("A"), 1, 0),
		E_INVAL);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	h_true(fk_call(SYS_PORT_LISTEN, fk_uptr("svc"), 3, 0) > 0, "nom libere");
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	connect_accept(void)
{
	t_process	*p;
	int64_t		l;
	int64_t		cli;
	int64_t		srv;
	int			i;

	fk_reset();
	p = fk_proc_new(PF_LISTEN);
	g_fk.cur = p;
	h_eq_i64("absent", fk_call(SYS_PORT_CONNECT, fk_uptr("s"), 1, 0), E_NOENT);
	l = fk_call(SYS_PORT_LISTEN, fk_uptr("svc"), 3, 0);
	h_eq_i64("rien en attente", fk_call(SYS_PORT_ACCEPT, l, 0, 0), E_TIMEOUT);
	cli = fk_call(SYS_PORT_CONNECT, fk_uptr("svc"), 3, 0);
	srv = fk_call(SYS_PORT_ACCEPT, l, TIMEOUT_INF, 0);
	h_true(cli > 0 && srv > 0, "connexion acceptee");
	h_eq_i64("accept canal", fk_call(SYS_PORT_ACCEPT, cli, 0, 0), E_BADF);
	fk_send(cli, &(t_chansend){fk_uptr("hi"), 2, 0, 0, 0});
	h_eq_i64("message client vers serveur", fk_recv(srv, &(t_chanrecv){
			fk_uptr(&i), 4, 0, 0, 0, 0, 0, 0, 0}), 0);
	i = 0;
	while (fk_call(SYS_PORT_CONNECT, fk_uptr("svc"), 3, 0) > 0)
		i++;
	h_eq_i64("64 connexions en attente", i, IPC_QUEUE_MAX);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	listener_close_pending(void)
{
	t_process	*p;
	int64_t		l;
	int64_t		cli;

	fk_reset();
	p = fk_proc_new(PF_LISTEN);
	g_fk.cur = p;
	l = fk_call(SYS_PORT_LISTEN, fk_uptr("svc"), 3, 0);
	cli = fk_call(SYS_PORT_CONNECT, fk_uptr("svc"), 3, 0);
	g_fk.heap_fail = 1;
	h_eq_i64("connexion sans memoire", fk_call(SYS_PORT_CONNECT,
			fk_uptr("svc"), 3, 0), E_NOMEM);
	g_fk.heap_fail = 3;
	h_eq_i64("canal sans memoire", fk_call(SYS_PORT_CONNECT,
			fk_uptr("svc"), 3, 0), E_NOMEM);
	fk_call(SYS_CLOSE, (uint64_t)l, 0, 0);
	h_eq_i64("serveur parti", fk_recv(cli, &(t_chanrecv){0, 0, 0, 0, 0,
			0, 0, 0, 0}), E_PIPE);
	h_eq_i64("ecouteur detruit", fk_call(SYS_PORT_CONNECT, fk_uptr("svc"),
			3, 0), E_NOENT);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/port");
	h_run("name_grammar", name_grammar);
	h_run("listen_rules", listen_rules);
	h_run("connect_accept", connect_accept);
	h_run("listener_close_pending", listener_close_pending);
	return (h_end());
}
