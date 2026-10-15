#include "time_int.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"

#define WAIT_TEST_NS 20000000ull
#define CANCEL_DELAY_NS 50000000ull

static bool	never(void *ctx)
{
	(void)ctx;
	return (false);
}

static void	unused_fire(void *ctx)
{
	(void)ctx;
	klog_err("time: minuterie annulée déclenchée quand même");
}

static int	test_cancel_wait(void)
{
	int64_t		id;
	uint64_t	t0;
	int			fails;

	id = timer_arm(time_now_ns() + CANCEL_DELAY_NS, unused_fire, NULL);
	fails = (id <= 0);
	fails += !timer_cancel(id);
	fails += timer_cancel(id);
	fails += timer_cancel(0) + timer_cancel(-5);
	fails += (timer_arm(0, NULL, NULL) != E_INVAL);
	fails += (wait_until(NULL, NULL, 1) != E_INVAL);
	t0 = time_now_ns();
	fails += (wait_until(never, NULL, WAIT_TEST_NS) != E_TIMEOUT);
	fails += (time_now_ns() - t0 < WAIT_TEST_NS);
	return (fails);
}

static void	power_option(void)
{
	const char	*op;

	op = boot_cmdline_get("power");
	if (!op)
		return ;
	klog_info("time: option power=%s de la ligne de commande", op);
	if (!strcmp(op, "off"))
		power_off();
	else if (!strcmp(op, "reboot"))
		power_reboot();
}

int	timer_selftest(void)
{
	uint64_t	a;
	uint64_t	b;
	int			fails;

	a = time_now_ns();
	b = time_now_ns();
	fails = (b < a || a == 0);
	fails += (time_wall_ns() < WALL_MIN_NS);
	fails += (time_set_wall_ns(WALL_MIN_NS - 1) != E_RANGE);
	fails += (time_set_wall_ns(WALL_MAX_NS) != E_RANGE);
	fails += test_cancel_wait();
	fails += timer_test_arm();
	if (fails)
		klog_err("time: autotest, %d échec(s)", fails);
	power_option();
	return (fails);
}
