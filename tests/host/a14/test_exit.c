#include <stdint.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

static void	call_exit_seven(void)
{
	exit(7);
}

static void	exit_order_and_code(void)
{
	fake_reset();
	fake_kernel_on();
	fs_h_reset();
	h_eq_i64("atexit 3", atexit(fs_h3), 0);
	h_eq_i64("atexit 2", atexit(fs_h2), 0);
	h_eq_i64("atexit handler qui enregistre", atexit(fs_h_reg), 0);
	h_eq_i64("code de sortie", fake_exit_catch(call_exit_seven), 7);
	h_eq_i64("4 handlers", fs_h_count(), 4);
	h_eq_i64("ordre inverse : le dernier enregistre d'abord", fs_h_at(0), 9);
	h_eq_i64("handler enregistre pendant exit", fs_h_at(1), 1);
	h_eq_i64("puis 2", fs_h_at(2), 2);
	h_eq_i64("puis 3", fs_h_at(3), 3);
}

static void	exit_overflow(void)
{
	int	i;

	fake_reset();
	fake_kernel_on();
	fs_h_reset();
	i = 0;
	while (i < ATEXIT_MAX)
	{
		h_eq_i64("enregistrement", atexit(fs_h_noop), 0);
		i++;
	}
	h_eq_i64("33e enregistrement refuse", atexit(fs_h_noop), -1);
	h_eq_i64("atexit(NULL) refuse", atexit(NULL), -1);
	h_eq_i64("exit(7)", fake_exit_catch(call_exit_seven), 7);
	h_eq_i64("32 handlers executes", fs_h_count(), ATEXIT_MAX);
}

int	main(void)
{
	h_begin("a14/exit");
	h_run("exit/transitions : ordre inverse et code", exit_order_and_code);
	h_run("exit/limites : table de 32", exit_overflow);
	return (h_end());
}
