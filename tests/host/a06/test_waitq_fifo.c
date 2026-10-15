#include <string.h>
#include "fake_sched.h"
#include "harness.h"

static t_fuwaiter	g_w[3];

static void	liste_fifo(void)
{
	t_waitq		wq;
	t_kthread	k[3];

	waitq_init(&wq);
	fu_kt_init(&k[0], 4);
	fu_kt_init(&k[1], 13);
	fu_kt_init(&k[2], 8);
	h_eq_i64("max d'une file vide", waitq_list_max_prio(&wq), -1);
	waitq_list_append(&wq, &k[0]);
	waitq_list_append(&wq, &k[1]);
	waitq_list_append(&wq, &k[2]);
	h_eq_i64("priorité max", waitq_list_max_prio(&wq), 13);
	waitq_list_remove(&wq, &k[1]);
	h_true(waitq_list_pop(&wq) == &k[0], "tête d'abord");
	h_true(waitq_list_pop(&wq) == &k[2], "puis la queue");
	h_true(waitq_list_pop(&wq) == NULL, "vide");
	h_true(wq.head == NULL && wq.tail == NULL, "tête et queue nulles");
}

static void	spawn_waiters(t_waitq *wq, void **h, int *order, int *n)
{
	int	i;

	i = 0;
	while (i < 3)
	{
		memset(&g_w[i], 0, sizeof(g_w[i]));
		g_w[i].wq = wq;
		g_w[i].timeout = TIMEOUT_NONE;
		g_w[i].prio = PRIO_NORMAL;
		g_w[i].tag = i;
		g_w[i].order = order;
		g_w[i].norder = n;
		h[i] = fh_spawn(fu_waiter, &g_w[i]);
		fu_wait_len(wq, (uint32_t)i + 1);
		i++;
	}
}

static void	reveil_un_fifo(void)
{
	t_waitq	wq;
	void	*h[3];
	int		order[3];
	int		n;
	int		i;

	waitq_init(&wq);
	n = 0;
	spawn_waiters(&wq, h, order, &n);
	i = 0;
	while (i < 3)
	{
		waitq_wake_one(&wq);
		fu_wait_flag(&n, i + 1);
		i++;
	}
	i = 0;
	while (i < 3)
		fh_join(h[i++]);
	h_true(n == 3 && order[0] == 0 && order[1] == 1 && order[2] == 2,
		"réveils dans l'ordre d'arrivée");
	h_true(g_w[0].rc == 0 && g_w[1].rc == 0 && g_w[2].rc == 0, "rc 0");
	h_eq_u64("file vide", fu_wq_len(&wq), 0);
}

static void	reveil_tous(void)
{
	t_waitq	wq;
	void	*h[3];
	int		order[3];
	int		n;
	int		i;

	waitq_init(&wq);
	n = 0;
	spawn_waiters(&wq, h, order, &n);
	waitq_wake_all(&wq);
	i = 0;
	while (i < 3)
		fh_join(h[i++]);
	h_eq_i64("trois réveillés", n, 3);
	h_true(g_w[0].rc == 0 && g_w[1].rc == 0 && g_w[2].rc == 0, "rc 0");
	h_eq_u64("file vide", fu_wq_len(&wq), 0);
	waitq_wake_all(&wq);
	waitq_wake_one(&wq);
	h_eq_u64("réveil d'une file vide sans effet", fu_wq_len(&wq), 0);
}

int	main(void)
{
	h_begin("a06/waitq_fifo");
	h_run("liste FIFO", liste_fifo);
	h_run("wake_one dans l'ordre", reveil_un_fifo);
	h_run("wake_all", reveil_tous);
	return (h_end());
}
