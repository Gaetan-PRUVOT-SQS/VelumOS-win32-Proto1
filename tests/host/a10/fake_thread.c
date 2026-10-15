#include "th.h"

void	*th_producer(void *arg)
{
	t_inpevent	ev;
	uint64_t	i;

	(void)arg;
	i = 0;
	while (i < th_pushes())
	{
		ev = th_event(INP_KEY_DOWN, (uint32_t)i + 1);
		ev.time_ns = 1;
		input_push(&ev);
		i++;
	}
	return (NULL);
}

static void	reader_step(t_rd *d)
{
	t_inpevent	ev[INPQ_TAKE_CHUNK];
	uint32_t	n;
	uint32_t	i;

	n = reader_take(d->r, ev, INPQ_TAKE_CHUNK);
	i = 0;
	while (i < n)
	{
		if (ev[i].code <= d->last)
			d->order_errors++;
		d->last = ev[i].code;
		i++;
	}
	d->taken += n;
}

void	*th_consumer(void *arg)
{
	t_rd	*d;

	d = (t_rd *)arg;
	while (d->last < th_pushes() && d->taken + d->r->lost < th_pushes())
		reader_step(d);
	return (NULL);
}
