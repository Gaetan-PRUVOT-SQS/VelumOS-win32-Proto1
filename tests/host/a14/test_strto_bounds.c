#include <limits.h>
#include "errno.h"
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

#define BOUNDS_N 25

static const t_f3_strtol	g_bounds[BOUNDS_N] = {
{"9223372036854775807", 10, LONG_MAX, 19, 0},
{"9223372036854775808", 10, LONG_MAX, 19, ERANGE},
{"-9223372036854775808", 10, LONG_MIN, 20, 0},
{"-9223372036854775809", 10, LONG_MIN, 20, ERANGE},
{"99999999999999999999999999", 10, LONG_MAX, 26, ERANGE},
{"-99999999999999999999999999", 10, LONG_MIN, 27, ERANGE},
{"18446744073709551615", 10, LONG_MAX, 20, ERANGE},
{"18446744073709551616", 10, LONG_MAX, 20, ERANGE},
{"0x7fffffffffffffff", 16, LONG_MAX, 18, 0},
{"0x8000000000000000", 16, LONG_MAX, 18, ERANGE},
{"-0x8000000000000000", 16, LONG_MIN, 19, 0},
{"-0x8000000000000001", 16, LONG_MIN, 19, ERANGE},
{"0x7fffffffffffffffg", 0, LONG_MAX, 18, 0},
{"9223372036854775807abc", 10, LONG_MAX, 19, 0},
{"9223372036854775808abc", 10, LONG_MAX, 19, ERANGE},
{"0", 10, 0, 1, 0},
{"1", 10, 1, 1, 0},
{"-1", 10, -1, 2, 0},
{"000000000000000000000000000000042", 10, 42, 33, 0},
{"10000000000000000000000000000000000000000", 10, LONG_MAX, 41, ERANGE},
{"-10000000000000000000000000000000000000000", 10, LONG_MIN, 42, ERANGE},
{"4611686018427387904", 10, 4611686018427387904, 19, 0},
{"-4611686018427387905", 10, -4611686018427387905, 20, 0},
{"1777777777777777777777", 8, LONG_MAX, 22, ERANGE},
{"777777777777777777777", 8, LONG_MAX, 21, 0},
};

static void	bounds_strtol(void)
{
	f3_strtol_run(g_bounds, BOUNDS_N, 0);
}

static void	bounds_strtoll(void)
{
	f3_strtol_run(g_bounds, BOUNDS_N, 1);
}

int	main(void)
{
	h_begin("a14/strto_bounds");
	h_run("strtol/limite : table bounds", bounds_strtol);
	h_run("strtoll/limite : table bounds", bounds_strtoll);
	return (h_end());
}
