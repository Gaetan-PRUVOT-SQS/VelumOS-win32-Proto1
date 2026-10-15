#include <string.h>
#include "fake_sched.h"
#include "harness.h"

static t_futest	g_t;

static void	stuck_holder(void *arg)
{
	t_futest	*t;

	t = arg;
	spin_lock(&t->lock);
	t->flag = 1;
	fu_wait_flag(&t->flag, 2);
	spin_unlock(&t->lock);
}

static void	spin_interblocage_une_seconde(void)
{
	void				*h;
	volatile uint64_t	start;

	spin_init(&g_t.lock, "bloqué");
	g_t.flag = 0;
	h = fh_spawn(stuck_holder, &g_t);
	fu_wait_flag(&g_t.flag, 1);
	start = fh_now_ns();
	if (setjmp(*fake_panic_arm()) == 0)
	{
		spin_lock(&g_t.lock);
		fake_panic_disarm();
		h_true(0, "attente de plus de 1 s : panique attendue");
	}
	else
	{
		h_true(fh_now_ns() - start >= SPIN_STUCK_NS, "au bout de 1 s");
		h_true(strstr(fake_panic_msg(), "interblocage") != NULL
			&& strstr(fake_panic_msg(), "bloqué") != NULL, "nom du verrou");
	}
	fake_reset_self();
	g_t.flag = 2;
	fh_join(h);
}

int	main(void)
{
	h_begin("a06/spin_stuck");
	h_run("interblocage détecté en 1 s", spin_interblocage_une_seconde);
	return (h_end());
}
