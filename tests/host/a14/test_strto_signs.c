#include <limits.h>
#include "errno.h"
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

#define SIGNS_N 33

static const t_f3_strtol	g_signs[SIGNS_N] = {
{"+5", 10, 5, 2, 0},
{"-5", 10, -5, 2, 0},
{"+-5", 10, 0, 0, 0},
{"-+5", 10, 0, 0, 0},
{"- 5", 10, 0, 0, 0},
{"+", 10, 0, 0, 0},
{"-", 10, 0, 0, 0},
{"", 10, 0, 0, 0},
{"   ", 10, 0, 0, 0},
{"  -42xyz", 10, -42, 5, 0},
{"-0", 10, 0, 2, 0},
{"+0x1f", 0, 31, 5, 0},
{"-0x1f", 0, -31, 5, 0},
{"-010", 0, -8, 4, 0},
{"abc", 10, 0, 0, 0},
{"12abc", 10, 12, 2, 0},
{"\t\n 7", 10, 7, 4, 0},
{"\v\f\r-7", 10, -7, 5, 0},
{" 1", 10, 1, 2, 0},
{"\t1", 10, 1, 2, 0},
{"\n1", 10, 1, 2, 0},
{"\v1", 10, 1, 2, 0},
{"\f1", 10, 1, 2, 0},
{"\r1", 10, 1, 2, 0},
{"\x01" "1", 10, 0, 0, 0},
{"\xa0" "1", 10, 0, 0, 0},
{"\x1f" "1", 10, 0, 0, 0},
{"\x7f" "1", 10, 0, 0, 0},
{"1 2", 10, 1, 1, 0},
{"1\t", 10, 1, 1, 0},
{"--1", 10, 0, 0, 0},
{"++1", 10, 0, 0, 0},
{"+ +1", 10, 0, 0, 0},
};

static void	signs_strtol(void)
{
	f3_strtol_run(g_signs, SIGNS_N, 0);
}

static void	signs_strtoll(void)
{
	f3_strtol_run(g_signs, SIGNS_N, 1);
}

static void	signs_errno_untouched(void)
{
	char	*end;

	errno = F3_SENTINEL;
	h_eq_i64("succes", strtol("12", &end, 10), 12);
	h_eq_i64("errno inchange apres succes", errno, F3_SENTINEL);
	h_eq_i64("aucun chiffre", strtol("zz", &end, 10), 0);
	h_eq_i64("errno inchange sans chiffre", errno, F3_SENTINEL);
	errno = 0;
	h_eq_i64("succes avec errno 0", strtol("99", &end, 10), 99);
	h_eq_i64("errno toujours 0", errno, 0);
}

int	main(void)
{
	h_begin("a14/strto_signs");
	h_run("strtol/partition : signes et espaces", signs_strtol);
	h_run("strtoll/partition : signes et espaces", signs_strtoll);
	h_run("strtol/supposition d'erreur : errno jamais remis a 0",
		signs_errno_untouched);
	return (h_end());
}
