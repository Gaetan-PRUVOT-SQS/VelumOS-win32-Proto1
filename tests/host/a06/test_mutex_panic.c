#include <string.h>
#include "fake_sched.h"
#include "harness.h"

static t_futest	g_t;

static void	mutex_paniques(void)
{
	mutex_init(&g_t.m, "double");
	mutex_lock(&g_t.m);
	if (setjmp(*fake_panic_arm()) == 0)
	{
		mutex_lock(&g_t.m);
		fake_panic_disarm();
		h_true(0, "verrouillage récursif : panique attendue");
	}
	else
		h_true(strstr(fake_panic_msg(), "récursif") != NULL, "récursif");
	fake_reset_self();
	mutex_init(&g_t.m, "étranger");
	if (setjmp(*fake_panic_arm()) == 0)
	{
		mutex_unlock(&g_t.m);
		fake_panic_disarm();
		h_true(0, "déverrouillage sans le tenir : panique attendue");
	}
	else
		h_true(strstr(fake_panic_msg(), "ne le tient pas") != NULL, "proprio");
	fake_reset_self();
}

int	main(void)
{
	h_begin("a06/mutex_panic");
	h_run("récursif et non-propriétaire", mutex_paniques);
	return (h_end());
}
