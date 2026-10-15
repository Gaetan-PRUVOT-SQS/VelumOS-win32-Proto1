#include "harness.h"
#include "th.h"

static void	start_readers(t_rd *rd, uint64_t *tc)
{
	int	i;

	i = 0;
	while (i < TH_READERS)
	{
		memset(&rd[i], 0, sizeof(rd[i]));
		th_open(INPUT_KIND_ALL, &rd[i].r);
		tc[i] = th_spawn(th_consumer, &rd[i]);
		i++;
	}
}

static void	finish_readers(t_rd *rd, uint64_t *tc)
{
	int	i;

	i = 0;
	while (i < TH_READERS)
	{
		th_join(tc[i]);
		h_eq_i64("ordre strictement croissant", rd[i].order_errors, 0);
		h_eq_u64("lus + perdus = produits", rd[i].taken + rd[i].r->lost,
			th_pushes());
		i++;
	}
}

static void	producer_and_readers(void)
{
	t_rd		rd[TH_READERS];
	uint64_t	tp;
	uint64_t	tc[TH_READERS];

	fake_all_reset();
	start_readers(rd, tc);
	tp = th_spawn(th_producer, NULL);
	th_join(tp);
	finish_readers(rd, tc);
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

int	main(void)
{
	h_begin("a10/queue_threads");
	h_run("1 producteur, 3 lecteurs concurrents", producer_and_readers);
	return (h_end());
}
