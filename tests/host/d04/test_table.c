#include <string.h>
#include "harness.h"
#include "fake.h"

static int	range_is(int lo, int hi, uint8_t fmt, uint8_t kind)
{
	int	bad;

	bad = 0;
	while (lo <= hi)
	{
		bad += (dexcode_format((uint8_t)lo) != fmt);
		bad += (dexcode_index_kind((uint8_t)lo) != kind);
		lo++;
	}
	return (bad == 0);
}

static void	table_lengths(void)
{
	int	op;
	int	unused;
	int	bad;

	op = 0;
	unused = 0;
	bad = 0;
	while (op < 256)
	{
		if (dexcode_format((uint8_t)op) == DF_NONE)
			unused++;
		else if (dexcode_format_len(dexcode_format((uint8_t)op)) < 1
			|| dexcode_format_len(dexcode_format((uint8_t)op)) > 5)
			bad++;
		op++;
	}
	h_eq_i64("codes inutilises", unused, 32);
	h_eq_i64("longueurs hors 1..5", bad, 0);
	h_eq_i64("format inconnu", dexcode_format_len(200), 0);
	h_eq_i64("10x", dexcode_format_len(DF_10X), 1);
	h_eq_i64("21c", dexcode_format_len(DF_21C), 2);
	h_eq_i64("35c", dexcode_format_len(DF_35C), 3);
	h_eq_i64("45cc", dexcode_format_len(DF_45CC), 4);
	h_eq_i64("51l", dexcode_format_len(DF_51L), 5);
}

static void	table_names(void)
{
	int	i;
	int	j;
	int	dup;

	i = 0;
	dup = 0;
	while (i < 256)
	{
		j = i + 1;
		while (j < 256)
			dup += !strcmp(dexcode_name((uint8_t)i),
					dexcode_name((uint8_t)j++));
		i++;
	}
	h_eq_i64("noms en double", dup, 0);
	h_eq_str("00", dexcode_name(OP_NOP), "nop");
	h_eq_str("18", dexcode_name(0x18), "const-wide");
	h_eq_str("90", dexcode_name(0x90), "add-int");
	h_eq_str("af", dexcode_name(0xaf), "rem-double");
	h_eq_str("b0", dexcode_name(0xb0), "add-int/2addr");
	h_eq_str("cf", dexcode_name(0xcf), "rem-double/2addr");
	h_eq_str("d1", dexcode_name(0xd1), "rsub-int");
	h_eq_str("e2", dexcode_name(0xe2), "ushr-int/lit8");
	h_eq_str("fb", dexcode_name(0xfb), "invoke-polymorphic/range");
	h_eq_i64("enum e2", OP_USHR_INT_LIT8, 0xe2);
}

static void	table_ranges(void)
{
	h_true(range_is(0x3e, 0x43, DF_NONE, DK_NONE), "3e..43 inutilises");
	h_true(range_is(0x73, 0x73, DF_NONE, DK_NONE), "73 inutilise");
	h_true(range_is(0x79, 0x7a, DF_NONE, DK_NONE), "79..7a inutilises");
	h_true(range_is(0xe3, 0xf9, DF_NONE, DK_NONE), "e3..f9 inutilises");
	h_true(range_is(0x2d, 0x31, DF_23X, DK_NONE), "cmp 23x");
	h_true(range_is(0x32, 0x37, DF_22T, DK_NONE), "if-test 22t");
	h_true(range_is(0x38, 0x3d, DF_21T, DK_NONE), "if-testz 21t");
	h_true(range_is(0x44, 0x51, DF_23X, DK_NONE), "arrayop 23x");
	h_true(range_is(0x52, 0x5f, DF_22C, DK_FIELD), "iinstanceop 22c");
	h_true(range_is(0x60, 0x6d, DF_21C, DK_FIELD), "sstaticop 21c");
	h_true(range_is(0x6e, 0x72, DF_35C, DK_METHOD), "invoke 35c");
	h_true(range_is(0x74, 0x78, DF_3RC, DK_METHOD), "invoke/range 3rc");
	h_true(range_is(0x7b, 0x8f, DF_12X, DK_NONE), "unop 12x");
	h_true(range_is(0x90, 0xaf, DF_23X, DK_NONE), "binop 23x");
	h_true(range_is(0xb0, 0xcf, DF_12X, DK_NONE), "binop/2addr 12x");
	h_true(range_is(0xd0, 0xd7, DF_22S, DK_NONE), "binop/lit16 22s");
	h_true(range_is(0xd8, 0xe2, DF_22B, DK_NONE), "binop/lit8 22b");
	h_true(range_is(0x1a, 0x1b, dexcode_format(0x1a), DK_STRING) == 0,
		"const-string et jumbo de formats differents");
	h_eq_i64("drapeaux goto", dexcode_flags(OP_GOTO), DO_BRANCH);
	h_eq_i64("drapeaux return-void", dexcode_flags(OP_RETURN_VOID), 0);
	h_eq_i64("drapeaux add-long", dexcode_flags(OP_ADD_LONG),
		DO_CONT | DO_WDST | DO_WB | DO_WC);
	h_eq_i64("drapeaux shl-long", dexcode_flags(OP_SHL_LONG),
		DO_CONT | DO_WDST | DO_WB);
}

int	main(void)
{
	h_begin("d04 table des codes");
	h_run("longueurs de format et codes inutilises", table_lengths);
	h_run("noms uniques", table_names);
	h_run("plages contigues de la specification", table_ranges);
	return (h_end());
}
