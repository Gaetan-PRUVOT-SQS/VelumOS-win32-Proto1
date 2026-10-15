#include <stdint.h>
#include "harness.h"
#include "stdlib.h"

static void	srand_distribution(void)
{
	int	count[16];
	int	i;
	int	bad;

	i = 0;
	while (i < 16)
		count[i++] = 0;
	srand(7);
	i = 0;
	while (i < 160000)
	{
		count[rand() >> 27]++;
		i++;
	}
	bad = 0;
	i = 0;
	while (i < 16)
	{
		bad += count[i] < 9500 || count[i] > 10500;
		i++;
	}
	h_eq_i64("16 tranches de bits hauts equilibrees a 5 %", bad, 0);
}

int	main(void)
{
	h_begin("a14/srand_dist");
	h_run("rand/metamorphique : repartition uniforme", srand_distribution);
	return (h_end());
}
