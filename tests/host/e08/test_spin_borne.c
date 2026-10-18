#include <string.h>
#include "fake_sched.h"
#include "harness.h"

static t_futest	g_t;

static void	bloqueur(void *arg)
{
	t_futest	*t;

	t = arg;
	spin_lock(&t->lock);
	t->flag = 1;
	fu_wait_flag(&t->flag, 2);
	spin_unlock(&t->lock);
}

static void	attente_active_bornee_sans_debug(void)
{
	void				*h;
	volatile uint64_t	debut;

	h_eq_u64("build sans VELUM_DEBUG", SCHED_DEBUG, 0);
	spin_init(&g_t.lock, "bloqué");
	g_t.flag = 0;
	h = fh_spawn(bloqueur, &g_t);
	fu_wait_flag(&g_t.flag, 1);
	debut = fh_now_ns();
	if (setjmp(*fake_panic_arm()) == 0)
	{
		spin_lock(&g_t.lock);
		fake_panic_disarm();
		h_true(0, "attente de plus de 1 s : panique attendue");
	}
	else
	{
		h_true(fh_now_ns() - debut >= SPIN_STUCK_NS, "au bout de 1 s");
		h_true(strstr(fake_panic_msg(), "interblocage") != NULL,
			"message d'interblocage");
	}
	fake_reset_self();
	g_t.flag = 2;
	fh_join(h);
}

int	main(void)
{
	h_begin("e08/spin_borne");
	h_run("interblocage détecté hors debug", attente_active_bornee_sans_debug);
	return (h_end());
}
