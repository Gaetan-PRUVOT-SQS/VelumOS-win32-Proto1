#include <stddef.h>
#include "harness.h"
#include "d09.h"

static const t_case	g_un[] = {
{"neg-int min", 0x7b, 0x80000000, 0, 0x80000000, 0},
{"not-int", 0x7c, 0, 0, 0xffffffff, 0},
{"neg-long min", 0x7d, 0x8000000000000000, 0, 0x8000000000000000, 0},
{"not-long", 0x7e, 0, 0, 0xffffffffffffffff, 0},
{"neg-float zero", 0x7f, 0, 0, 0x80000000, 0},
{"neg-double zero", 0x80, 0, 0, 0x8000000000000000, 0},
{"int-to-long -1", 0x81, 0xffffffff, 0, 0xffffffffffffffff, 0},
{"int-to-float max", 0x82, 0x7fffffff, 0, 0x4f000000, 0},
{"int-to-double -1", 0x83, 0xffffffff, 0, 0xbff0000000000000, 0},
{"long-to-int", 0x84, 0x1ffffffff, 0, 0xffffffff, 0},
{"long-to-float 1", 0x85, 1, 0, 0x3f800000, 0},
{"long-to-double -1", 0x86, 0xffffffffffffffff, 0, 0xbff0000000000000, 0},
{"float-to-int nan", 0x87, 0x7fc00000, 0, 0, 0},
{"float-to-int +inf", 0x87, 0x7f800000, 0, 0x7fffffff, 0},
{"float-to-int -inf", 0x87, 0xff800000, 0, 0x80000000, 0},
{"float-to-int 2^31", 0x87, 0x4f000000, 0, 0x7fffffff, 0},
{"float-to-int -2^31", 0x87, 0xcf000000, 0, 0x80000000, 0},
{"float-to-int -1.5", 0x87, 0xbfc00000, 0, 0xffffffff, 0},
{"float-to-long nan", 0x88, 0x7fc00000, 0, 0, 0},
{"float-to-long +inf", 0x88, 0x7f800000, 0, 0x7fffffffffffffff, 0},
{"float-to-long 2^63", 0x88, 0x5f000000, 0, 0x7fffffffffffffff, 0},
{"float-to-long -2^63", 0x88, 0xdf000000, 0, 0x8000000000000000, 0},
{"float-to-long -1.5", 0x88, 0xbfc00000, 0, 0xffffffffffffffff, 0},
{"float-to-double 1.5", 0x89, 0x3fc00000, 0, 0x3ff8000000000000, 0},
{"double-to-int nan", 0x8a, 0x7ff8000000000000, 0, 0, 0},
{"double-to-int +inf", 0x8a, 0x7ff0000000000000, 0, 0x7fffffff, 0},
{"double-to-int -inf", 0x8a, 0xfff0000000000000, 0, 0x80000000, 0},
{"double-to-int max+0.5", 0x8a, 0x41dfffffffe00000, 0, 0x7fffffff, 0},
{"double-to-int min-0.5", 0x8a, 0xc1e0000000100000, 0, 0x80000000, 0},
{"double-to-int -0.9", 0x8a, 0xbfeccccccccccccd, 0, 0, 0},
{"double-to-long nan", 0x8b, 0x7ff8000000000000, 0, 0, 0},
{"double-to-long +inf", 0x8b, 0x7ff0000000000000, 0, 0x7fffffffffffffff, 0},
{"double-to-long -inf", 0x8b, 0xfff0000000000000, 0, 0x8000000000000000, 0},
{"double-to-long 2^63", 0x8b, 0x43e0000000000000, 0, 0x7fffffffffffffff, 0},
{"double-to-long -2^63", 0x8b, 0xc3e0000000000000, 0, 0x8000000000000000,
	0},
{"double-to-long -1.5", 0x8b, 0xbff8000000000000, 0, 0xffffffffffffffff, 0},
{"double-to-float 1.5", 0x8c, 0x3ff8000000000000, 0, 0x3fc00000, 0},
{"double-to-float 1e300", 0x8c, 0x7e37e43c8800759c, 0, 0x7f800000, 0},
{"int-to-byte 0x180", 0x8d, 0x180, 0, 0xffffff80, 0},
{"int-to-byte 0x7f", 0x8d, 0x7f, 0, 0x7f, 0},
{"int-to-char -1", 0x8e, 0xffffffff, 0, 0xffff, 0},
{"int-to-short 0x18000", 0x8f, 0x18000, 0, 0xffff8000, 0},
{NULL, 0, 0, 0, 0, 0}
};

static const t_case	g_cmp[] = {
{"cmpl-float nan", 0x2d, 0x7fc00000, 0x3f800000, 0xffffffff, 0},
{"cmpg-float nan", 0x2e, 0x7fc00000, 0x3f800000, 1, 0},
{"cmpl-float 1<2", 0x2d, 0x3f800000, 0x40000000, 0xffffffff, 0},
{"cmpg-float 2>1", 0x2e, 0x40000000, 0x3f800000, 1, 0},
{"cmpl-float egal", 0x2d, 0x3f800000, 0x3f800000, 0, 0},
{"cmpl-double nan", 0x2f, 0x3ff0000000000000, 0x7ff8000000000000,
	0xffffffff, 0},
{"cmpg-double nan", 0x30, 0x3ff0000000000000, 0x7ff8000000000000, 1, 0},
{"cmpg-double -0 et +0", 0x30, 0x8000000000000000, 0, 0, 0},
{"cmp-long min<max", 0x31, 0x8000000000000000, 0x7fffffffffffffff,
	0xffffffff, 0},
{"cmp-long egal", 0x31, 5, 5, 0, 0},
{"cmp-long 1>0", 0x31, 1, 0, 1, 0},
{NULL, 0, 0, 0, 0, 0}
};

static void	unaires_et_conversions(void)
{
	fk_table(g_un, fk_case2);
}

static void	comparaisons(void)
{
	fk_table(g_cmp, fk_case3);
}

int	main(void)
{
	h_begin("d09/conv");
	h_run("unaires_et_conversions", unaires_et_conversions);
	h_run("comparaisons", comparaisons);
	return (h_end());
}
