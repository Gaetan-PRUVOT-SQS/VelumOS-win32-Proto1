#include <limits.h>
#include "errno.h"
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

#define UNSIGNED_N 25

static const t_f3_strtoul	g_unsigned[UNSIGNED_N] = {
{"18446744073709551615", 10, ULLONG_MAX, 20, 0},
{"18446744073709551616", 10, ULLONG_MAX, 20, ERANGE},
{"-1", 10, ULLONG_MAX, 2, 0},
{"-0", 10, 0, 2, 0},
{"+1", 10, 1, 2, 0},
{"-2", 10, ULLONG_MAX - 1, 2, 0},
{"-18446744073709551615", 10, 1, 21, 0},
{"-18446744073709551616", 10, ULLONG_MAX, 21, ERANGE},
{"0xffffffffffffffff", 16, ULLONG_MAX, 18, 0},
{"0x10000000000000000", 16, ULLONG_MAX, 19, ERANGE},
{"99999999999999999999999999", 10, ULLONG_MAX, 26, ERANGE},
{"0", 10, 0, 1, 0},
{"abc", 10, 0, 0, 0},
{"123abc", 10, 123, 3, 0},
{"9223372036854775808", 10, 9223372036854775808ULL, 19, 0},
{"1777777777777777777777", 8, ULLONG_MAX, 22, 0},
{"2000000000000000000000", 8, ULLONG_MAX, 22, ERANGE},
{"1", 1, 0, 0, EINVAL},
{"1", 37, 0, 0, EINVAL},
{"  \t42", 10, 42, 5, 0},
{"-", 10, 0, 0, 0},
{"", 10, 0, 0, 0},
{"0x", 0, 0, 1, 0},
{"0xzz", 16, 0, 1, 0},
{"zz", 36, 1295, 2, 0},
};

static void	unsigned_strtoul(void)
{
	f3_strtoul_run(g_unsigned, UNSIGNED_N, 0);
}

static void	unsigned_strtoull(void)
{
	f3_strtoul_run(g_unsigned, UNSIGNED_N, 1);
}

static void	unsigned_null_endptr(void)
{
	errno = F3_SENTINEL;
	h_eq_u64("strtoul, endptr NULL", strtoul("77", NULL, 10), 77);
	h_eq_u64("strtoull, endptr NULL", strtoull("0xff", NULL, 0), 255);
	h_eq_u64("strtoull, base invalide", strtoull("5", NULL, 1), 0);
	h_eq_i64("errno EINVAL", errno, EINVAL);
}

int	main(void)
{
	h_begin("a14/strto_unsigned");
	h_run("strtoul/limite : table", unsigned_strtoul);
	h_run("strtoull/limite : table", unsigned_strtoull);
	h_run("strtoul/partition : endptr NULL", unsigned_null_endptr);
	return (h_end());
}
