#include <pthread.h>
#include <stdint.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/vsync.h"

#define SPIN_THREADS 4
#define SPIN_LOOPS 100000

static void	spin_basic(void)
{
	t_vspin	s;

	fake_reset();
	s.locked = 0;
	v_spin_lock(&s);
	h_eq_i64("trylock sur verrou pris", v_spin_trylock(&s), 0);
	v_spin_unlock(&s);
	h_eq_i64("trylock sur verrou libre", v_spin_trylock(&s), 1);
	h_eq_i64("verrou repris", v_spin_trylock(&s), 0);
	v_spin_unlock(&s);
	h_eq_u64("etat libre", s.locked, 0);
}

static void	spin_contention(void)
{
	pthread_t	th[SPIN_THREADS];
	t_fsjob		job[SPIN_THREADS];
	t_vspin		s;
	uint64_t	counter;
	int			i;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	s.locked = 0;
	counter = 0;
	i = -1;
	while (++i < SPIN_THREADS)
	{
		job[i].spin = &s;
		job[i].counter = &counter;
		job[i].loops = SPIN_LOOPS;
		pthread_create(&th[i], NULL, fs_spin_worker, &job[i]);
	}
	while (i--)
		pthread_join(th[i], NULL);
	h_eq_u64("compteur exact", counter, (uint64_t)SPIN_THREADS * SPIN_LOOPS);
}

int	main(void)
{
	h_begin("a14/spin");
	h_run("spin/etats : lock, trylock, unlock", spin_basic);
	h_run("spin/concurrence : 4 fils x 100 000", spin_contention);
	return (h_end());
}
