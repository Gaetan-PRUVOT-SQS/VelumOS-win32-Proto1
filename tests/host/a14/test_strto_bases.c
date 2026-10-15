#include <limits.h>
#include "errno.h"
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

#define BASES_N 50

static const t_f3_strtol	g_bases[BASES_N] = {
{"0", 0, 0, 1, 0},
{"7", 0, 7, 1, 0},
{"0755", 0, 493, 4, 0},
{"08", 0, 0, 1, 0},
{"0x1f", 0, 31, 4, 0},
{"0X1F", 0, 31, 4, 0},
{"0x", 0, 0, 1, 0},
{"0xg", 0, 0, 1, 0},
{"0x1g", 0, 1, 3, 0},
{"123", 0, 123, 3, 0},
{"010", 0, 8, 3, 0},
{"0", 8, 0, 1, 0},
{"1010", 2, 10, 4, 0},
{"102", 2, 2, 2, 0},
{"2", 2, 0, 0, 0},
{"0b1", 2, 0, 1, 0},
{"777", 8, 511, 3, 0},
{"0777", 8, 511, 4, 0},
{"8", 8, 0, 0, 0},
{"789", 8, 7, 1, 0},
{"0x10", 10, 0, 1, 0},
{"12", 10, 12, 2, 0},
{"ff", 16, 255, 2, 0},
{"FF", 16, 255, 2, 0},
{"0xff", 16, 255, 4, 0},
{"0xFf", 16, 255, 4, 0},
{"0x", 16, 0, 1, 0},
{"0xg", 16, 0, 1, 0},
{"g", 16, 0, 0, 0},
{"1g", 16, 1, 1, 0},
{"2102", 3, 65, 4, 0},
{"3", 3, 0, 0, 0},
{"z", 36, 35, 1, 0},
{"Z", 36, 35, 1, 0},
{"zz", 36, 1295, 2, 0},
{"ZZ", 36, 1295, 2, 0},
{"Zz", 36, 1295, 2, 0},
{"10", 36, 36, 2, 0},
{"-zz", 36, -1295, 3, 0},
{"zz!", 36, 1295, 2, 0},
{"z", 35, 0, 0, 0},
{"y", 35, 34, 1, 0},
{"123", 1, 0, 0, EINVAL},
{"123", 37, 0, 0, EINVAL},
{"123", -1, 0, 0, EINVAL},
{"123", 100, 0, 0, EINVAL},
{"123", 2147483647, 0, 0, EINVAL},
{"", 1, 0, 0, EINVAL},
{"0x1f", 37, 0, 0, EINVAL},
{"  12", 99, 0, 0, EINVAL},
};

static void	bases_strtol(void)
{
	f3_strtol_run(g_bases, BASES_N, 0);
}

static void	bases_strtoll(void)
{
	f3_strtol_run(g_bases, BASES_N, 1);
}

static void	bases_null_endptr(void)
{
	errno = F3_SENTINEL;
	h_eq_i64("endptr NULL, base 10", strtol("123", NULL, 10), 123);
	h_eq_i64("endptr NULL, base 0 hexa", strtol("0x7f", NULL, 0), 127);
	h_eq_i64("endptr NULL, base invalide", strtol("5", NULL, 99), 0);
	h_eq_i64("errno EINVAL", errno, EINVAL);
	errno = F3_SENTINEL;
	h_eq_i64("endptr NULL, aucun chiffre", strtol("x", NULL, 10), 0);
	h_eq_i64("errno inchange", errno, F3_SENTINEL);
}

int	main(void)
{
	h_begin("a14/strto_bases");
	h_run("strtol/partition : bases et prefixes", bases_strtol);
	h_run("strtoll/partition : bases et prefixes", bases_strtoll);
	h_run("strtol/partition : endptr NULL", bases_null_endptr);
	return (h_end());
}
