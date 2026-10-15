#include <string.h>
#include "fake_sched.h"
#include "harness.h"

#define FIFO_ROUNDS 50

static void	fifo_taker(void *arg)
{
	t_fuwaiter	*w;

	w = arg;
	spin_lock(w->held);
	w->order[(*w->norder)++] = w->tag;
	spin_unlock(w->held);
}

static void	*fifo_queue(t_fuwaiter *w, int tag, int *order, int *n)
{
	void		*h;
	uint32_t	want;

	w->tag = tag;
	w->order = order;
	w->norder = n;
	want = __atomic_load_n(&w->held->ticket, __ATOMIC_ACQUIRE) + 1;
	h = fh_spawn(fifo_taker, w);
	while (__atomic_load_n(&w->held->ticket, __ATOMIC_ACQUIRE) != want)
		fh_sleep_us(20);
	return (h);
}

static int	fifo_round(void)
{
	t_spinlock	l;
	t_fuwaiter	wb;
	t_fuwaiter	wc;
	int			rec[3];
	void		*h[2];

	spin_init(&l, "fifo");
	memset(&wb, 0, sizeof(wb));
	memset(&wc, 0, sizeof(wc));
	wb.held = &l;
	wc.held = &l;
	rec[2] = 0;
	spin_lock(&l);
	h[0] = fifo_queue(&wb, 1, rec, &rec[2]);
	h[1] = fifo_queue(&wc, 2, rec, &rec[2]);
	spin_unlock(&l);
	fh_join(h[0]);
	fh_join(h[1]);
	return (rec[2] == 2 && rec[0] == 1 && rec[1] == 2);
}

static void	spin_ordre_des_tickets(void)
{
	int	ok;
	int	i;

	ok = 0;
	i = 0;
	while (i < FIFO_ROUNDS)
	{
		ok += fifo_round();
		i++;
	}
	h_eq_i64("50 tours servis dans l'ordre des tickets", ok, FIFO_ROUNDS);
}

int	main(void)
{
	t_spinlock	l;

	h_begin("a06/spin_fifo");
	h_run("ordre FIFO des tickets", spin_ordre_des_tickets);
	spin_init(&l, "libre");
	if (setjmp(*fake_panic_arm()) == 0)
	{
		spin_unlock(&l);
		fake_panic_disarm();
		h_true(0, "spin_unlock d'un verrou libre doit paniquer");
	}
	else
		h_true(strstr(fake_panic_msg(), "non pris") != NULL, "message");
	fake_reset_self();
	return (h_end());
}
