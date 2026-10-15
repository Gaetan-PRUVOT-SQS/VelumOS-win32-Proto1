#include <stddef.h>
#include "harness.h"
#include "d09.h"

static const t_case	g_int[] = {
{"add-int max+1", 0x90, 0x7fffffff, 1, 0x80000000, 0},
{"sub-int min-1", 0x91, 0x80000000, 1, 0x7fffffff, 0},
{"mul-int deborde", 0x92, 100000, 100000, 1410065408, 0},
{"div-int 7/2", 0x93, 7, 2, 3, 0},
{"div-int -7/2", 0x93, 0xfffffff9, 2, 0xfffffffd, 0},
{"div-int min/-1", 0x93, 0x80000000, 0xffffffff, 0x80000000, 0},
{"div-int zero", 0x93, 1, 0, 0, DVM_THROWN},
{"rem-int -7%2", 0x94, 0xfffffff9, 2, 0xffffffff, 0},
{"rem-int min%-1", 0x94, 0x80000000, 0xffffffff, 0, 0},
{"rem-int zero", 0x94, 1, 0, 0, DVM_THROWN},
{"and-int", 0x95, 0xf0f0, 0xff00, 0xf000, 0},
{"or-int", 0x96, 0xf0f0, 0xff00, 0xfff0, 0},
{"xor-int", 0x97, 0xf0f0, 0xff00, 0x0ff0, 0},
{"shl-int 31", 0x98, 1, 31, 0x80000000, 0},
{"shl-int 33 masque", 0x98, 1, 33, 2, 0},
{"shr-int 31", 0x99, 0x80000000, 31, 0xffffffff, 0},
{"shr-int 32 masque", 0x99, 0x80000000, 32, 0x80000000, 0},
{"ushr-int 31", 0x9a, 0x80000000, 31, 1, 0},
{"ushr-int 32 masque", 0x9a, 0xffffffff, 32, 0xffffffff, 0},
{NULL, 0, 0, 0, 0, 0}
};

static const t_case	g_long[] = {
{"add-long max+1", 0x9b, 0x7fffffffffffffff, 1, 0x8000000000000000, 0},
{"sub-long min-1", 0x9c, 0x8000000000000000, 1, 0x7fffffffffffffff, 0},
{"mul-long deborde", 0x9d, 0x100000000, 0x100000000, 0, 0},
{"div-long -7/2", 0x9e, 0xfffffffffffffff9, 2, 0xfffffffffffffffd, 0},
{"div-long min/-1", 0x9e, 0x8000000000000000, 0xffffffffffffffff,
	0x8000000000000000, 0},
{"div-long zero", 0x9e, 1, 0, 0, DVM_THROWN},
{"rem-long -7%2", 0x9f, 0xfffffffffffffff9, 2, 0xffffffffffffffff, 0},
{"rem-long min%-1", 0x9f, 0x8000000000000000, 0xffffffffffffffff, 0, 0},
{"rem-long zero", 0x9f, 1, 0, 0, DVM_THROWN},
{"and-long", 0xa0, 0xf0f0f0f0f0f0f0f0, 0xff00ff00ff00ff00,
	0xf000f000f000f000, 0},
{"or-long", 0xa1, 0xf0, 0x100000000, 0x1000000f0, 0},
{"xor-long", 0xa2, 0xffffffffffffffff, 0xffff, 0xffffffffffff0000, 0},
{"shl-long 63", 0xa3, 1, 63, 0x8000000000000000, 0},
{"shl-long 65 masque", 0xa3, 1, 65, 2, 0},
{"shr-long 63", 0xa4, 0x8000000000000000, 63, 0xffffffffffffffff, 0},
{"shr-long 64 masque", 0xa4, 0x8000000000000000, 64, 0x8000000000000000, 0},
{"ushr-long 63", 0xa5, 0x8000000000000000, 63, 1, 0},
{NULL, 0, 0, 0, 0, 0}
};

static const t_case	g_addr[] = {
{"add-int/2addr", 0xb0, 0x7fffffff, 1, 0x80000000, 0},
{"div-int/2addr zero", 0xb3, 5, 0, 0, DVM_THROWN},
{"rem-int/2addr", 0xb4, 17, 5, 2, 0},
{"shl-int/2addr 33", 0xb8, 1, 33, 2, 0},
{"add-long/2addr", 0xbb, 0xffffffff, 1, 0x100000000, 0},
{"div-long/2addr min/-1", 0xbe, 0x8000000000000000, 0xffffffffffffffff,
	0x8000000000000000, 0},
{"shl-long/2addr 65", 0xc3, 1, 65, 2, 0},
{"mul-float/2addr", 0xc8, 0x40000000, 0x40400000, 0x40c00000, 0},
{"rem-float/2addr", 0xca, 0x40b00000, 0x40000000, 0x3fc00000, 0},
{"mul-double/2addr", 0xcd, 0x4000000000000000, 0x4008000000000000,
	0x4018000000000000, 0},
{"rem-double/2addr", 0xcf, 0x4016000000000000, 0x4000000000000000,
	0x3ff8000000000000, 0},
{NULL, 0, 0, 0, 0, 0}
};

static const t_case	g_lit[] = {
{"add-int/lit8 max+1", 0xd8, 0x7fffffff, 1, 0x80000000, 0},
{"rsub-int/lit8 10-3", 0xd9, 3, 10, 7, 0},
{"mul-int/lit8 -3*-2", 0xda, 0xfffffffd, 0xfe, 6, 0},
{"div-int/lit8 -7/2", 0xdb, 0xfffffff9, 2, 0xfffffffd, 0},
{"div-int/lit8 min/-1", 0xdb, 0x80000000, 0xff, 0x80000000, 0},
{"div-int/lit8 zero", 0xdb, 1, 0, 0, DVM_THROWN},
{"rem-int/lit8 -7%2", 0xdc, 0xfffffff9, 2, 0xffffffff, 0},
{"rem-int/lit8 zero", 0xdc, 1, 0, 0, DVM_THROWN},
{"and-int/lit8", 0xdd, 0xff, 0x0f, 0x0f, 0},
{"or-int/lit8 -128", 0xde, 1, 0x80, 0xffffff81, 0},
{"xor-int/lit8 -1", 0xdf, 0, 0xff, 0xffffffff, 0},
{"shl-int/lit8 33", 0xe0, 1, 33, 2, 0},
{"shr-int/lit8", 0xe1, 0xfffffff8, 1, 0xfffffffc, 0},
{"ushr-int/lit8", 0xe2, 0xfffffff8, 28, 0xf, 0},
{"add-int/lit16 -32768", 0xd0, 1, 0x8000, 0xffff8001, 0},
{"rsub-int 32767-1", 0xd1, 1, 0x7fff, 0x7ffe, 0},
{"mul-int/lit16", 0xd2, 3, 1000, 3000, 0},
{"div-int/lit16", 0xd3, 100, 7, 14, 0},
{"div-int/lit16 zero", 0xd3, 100, 0, 0, DVM_THROWN},
{"rem-int/lit16", 0xd4, 100, 7, 2, 0},
{"and-int/lit16", 0xd5, 0x12345, 0x0ff0, 0x0340, 0},
{"or-int/lit16", 0xd6, 0x10000, 0x00ff, 0x100ff, 0},
{"xor-int/lit16 -1", 0xd7, 0, 0xffff, 0xffffffff, 0},
{NULL, 0, 0, 0, 0, 0}
};

static void	entiers_trois_registres(void)
{
	fk_table(g_int, fk_case3);
}

static void	longs_trois_registres(void)
{
	fk_table(g_long, fk_case3);
}

static void	deux_adresses(void)
{
	fk_table(g_addr, fk_case2a);
}

static void	litteraux(void)
{
	fk_table(g_lit, fk_caselit);
}

int	main(void)
{
	h_begin("d09/arith");
	h_run("entiers_trois_registres", entiers_trois_registres);
	h_run("longs_trois_registres", longs_trois_registres);
	h_run("deux_adresses", deux_adresses);
	h_run("litteraux", litteraux);
	return (h_end());
}
