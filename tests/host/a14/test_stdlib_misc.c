#include <limits.h>
#include "errno.h"
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

static void	misc_abs(void)
{
	h_eq_i64("abs(0)", abs(0), 0);
	h_eq_i64("abs(5)", abs(5), 5);
	h_eq_i64("abs(-5)", abs(-5), 5);
	h_eq_i64("abs(INT_MAX)", abs(INT_MAX), INT_MAX);
	h_eq_i64("abs(-INT_MAX)", abs(-INT_MAX), INT_MAX);
	h_eq_i64("abs(INT_MIN) sans comportement indefini", abs(INT_MIN), INT_MIN);
}

static void	misc_labs_llabs(void)
{
	h_eq_i64("labs(5)", labs(5), 5);
	h_eq_i64("labs(-7)", labs(-7), 7);
	h_eq_i64("labs(LONG_MAX)", labs(LONG_MAX), LONG_MAX);
	h_eq_i64("labs(-LONG_MAX)", labs(-LONG_MAX), LONG_MAX);
	h_eq_i64("labs(LONG_MIN) sans comportement indefini", labs(LONG_MIN),
		LONG_MIN);
	h_eq_i64("llabs(0), llabs(5)", llabs(0) + llabs(5), 5);
	h_eq_i64("llabs(-7)", llabs(-7), 7);
	h_eq_i64("llabs(-LLONG_MAX)", llabs(-LLONG_MAX), LLONG_MAX);
	h_eq_i64("llabs(LLONG_MIN) sans comportement indefini", llabs(LLONG_MIN),
		LLONG_MIN);
}

static void	misc_atoi(void)
{
	h_eq_i64("atoi 42", atoi("42"), 42);
	h_eq_i64("atoi espaces et signe", atoi("  -17"), -17);
	h_eq_i64("atoi +8", atoi("+8"), 8);
	h_eq_i64("atoi chaine vide", atoi(""), 0);
	h_eq_i64("atoi sans chiffre", atoi("abc"), 0);
	h_eq_i64("atoi suffixe ignore", atoi("12abc"), 12);
	h_eq_i64("atoi INT_MAX", atoi("2147483647"), INT_MAX);
	h_eq_i64("atoi INT_MIN", atoi("-2147483648"), INT_MIN);
}

static void	misc_atol(void)
{
	errno = F3_SENTINEL;
	h_eq_i64("atol LONG_MAX", atol("9223372036854775807"), LONG_MAX);
	h_eq_i64("atol LONG_MIN", atol("-9223372036854775808"), LONG_MIN);
	h_eq_i64("errno inchange", errno, F3_SENTINEL);
	h_eq_i64("atol depasse : LONG_MAX", atol("99999999999999999999"), LONG_MAX);
	h_eq_i64("errno ERANGE", errno, ERANGE);
	h_eq_i64("atol vide", atol(""), 0);
	h_eq_i64("atol espaces", atol(" \t 77"), 77);
}

int	main(void)
{
	h_begin("a14/stdlib_misc");
	h_run("abs/limite : INT_MIN, INT_MAX", misc_abs);
	h_run("labs, llabs/limite : MIN, MAX", misc_labs_llabs);
	h_run("atoi/partition : signes, espaces, suffixes", misc_atoi);
	h_run("atol/limite : bornes et errno", misc_atol);
	return (h_end());
}
