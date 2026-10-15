#include <pthread.h>
#include <stdint.h>
#include "a04_fake.h"

static void	*worker(void *arg)
{
	t_work		*w;
	uint64_t	flags;
	uint32_t	i;

	w = arg;
	i = 0;
	while (i < 2000)
	{
		flags = heap_lock(&w->lock);
		w->counter = w->counter + 1;
		heap_unlock(&w->lock, flags);
		i++;
	}
	return (NULL);
}

static void	lock_ticket_and_irq_balance(void)
{
	t_spinlock	l;
	uint64_t	flags;

	heap_lock_init(&l, "test");
	flags = heap_lock(&l);
	h_eq_u64("E13 ticket pris", l.ticket, 1);
	h_eq_u64("E13 service inchange", l.serving, 0);
	h_eq_i64("E13 interruptions coupees", fake_irq_depth(), 1);
	heap_unlock(&l, flags);
	h_eq_u64("E13 service avance", l.serving, 1);
	h_eq_i64("E13 interruptions rendues", fake_irq_depth(), 0);
	flags = heap_lock(&l);
	heap_unlock(&l, flags);
	h_eq_u64("E13 second tour : ticket", l.ticket, 2);
	h_eq_u64("E13 second tour : service", l.serving, 2);
}

static void	lock_mutual_exclusion(void)
{
	pthread_t	th[4];
	t_work		w;
	int			i;

	heap_lock_init(&w.lock, "test.mutex");
	w.counter = 0;
	i = 0;
	while (i < 4)
	{
		pthread_create(&th[i], NULL, worker, &w);
		i++;
	}
	i = 0;
	while (i < 4)
		pthread_join(th[i++], NULL);
	h_eq_u64("E13 exclusion mutuelle : somme exacte", w.counter,
		4ull * 2000);
	h_eq_u64("E13 tickets = services", w.lock.ticket, w.lock.serving);
	h_eq_i64("E13 equilibre apres les fils", fake_irq_errors(), 0);
}

int	main(void)
{
	h_begin("a04/verrou");
	h_run("E13 ticket et interruptions", lock_ticket_and_irq_balance);
	h_run("E13 exclusion mutuelle sur 4 fils", lock_mutual_exclusion);
	return (h_end());
}
