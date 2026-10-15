#include <stdint.h>
#include <stdio.h>
#include "a04_fake.h"

static void	concurrent_random_operations(void)
{
	printf("a04 concurrent : %d fils, graine 0xa04\n", A04_THREADS);
	a04_conc_run(0xa04, 3000);
}

static void	concurrent_another_seed(void)
{
	printf("a04 concurrent : %d fils, graine 0x5ca1ab1e\n", A04_THREADS);
	a04_conc_run(0x5ca1ab1e, 3000);
}

int	main(void)
{
	h_begin("a04/concurrence");
	h_run("E11 4 fils : liberations croisees", concurrent_random_operations);
	h_run("E11 4 fils : autre graine", concurrent_another_seed);
	return (h_end());
}
