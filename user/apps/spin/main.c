#include <string.h>
#include "velum/velum.h"
#include "spin.h"

static void	*ticker(void *arg)
{
	int	i;

	(void)arg;
	i = 0;
	while (i < 3)
	{
		v_sleep(100000000ull);
		v_log(V_LOG_INFO, "SPIN tick");
		i++;
	}
	v_log(V_LOG_INFO, "SPIN PASS");
	return (NULL);
}

int	main(int argc, char **argv)
{
	t_vthread	*t;

	if (argc > 1 && !strcmp(argv[1], "loop"))
		while (1)
			;
	v_log(V_LOG_INFO, "SPIN start");
	if (v_thread_create(&t, ticker, NULL) < 0)
	{
		v_log(V_LOG_ERR, "SPIN FAIL thread");
		return (1);
	}
	spin_kill_test(argv[0]);
	while (1)
		;
	return (0);
}
