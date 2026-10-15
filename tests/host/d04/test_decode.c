#include "harness.h"
#include "fake.h"

static void	decode_one_unit(void)
{
	t_dinsn	in;

	in = fake_dec(0x0000, 0);
	h_eq_i64("10x len", in.len, 1);
	h_eq_i64("10x fmt", in.fmt, DF_10X);
	in = fake_dec(0xef01, 0);
	h_eq_i64("12x A", in.a, 15);
	h_eq_i64("12x B", in.b, 14);
	in = fake_dec(0x8f12, 0);
	h_eq_i64("11n A max", in.a, 15);
	h_eq_i64("11n litteral min", in.lit, -8);
	in = fake_dec(0x7012, 0);
	h_eq_i64("11n litteral max", in.lit, 7);
	in = fake_dec(0xff0f, 0);
	h_eq_i64("11x AA max", in.a, 255);
	in = fake_dec(0x8028, 0);
	h_eq_i64("10t deplacement min", in.lit, -128);
	in = fake_dec(0x7f28, 0);
	h_eq_i64("10t deplacement max", in.lit, 127);
}

static void	decode_two_units(void)
{
	t_dinsn	in;

	in = fake_dec(0x80000029, 0);
	h_eq_i64("20t min", in.lit, -32768);
	h_eq_i64("20t len", in.len, 2);
	in = fake_dec(0xffffff02, 0);
	h_eq_i64("22x AA", in.a, 255);
	h_eq_i64("22x BBBB", in.b, 65535);
	in = fake_dec(0x8000ff38, 0);
	h_eq_i64("21t AA", in.a, 255);
	h_eq_i64("21t min", in.lit, -32768);
	in = fake_dec(0x7fffff13, 0);
	h_eq_i64("21s max", in.lit, 32767);
	in = fake_dec(0x8000ff13, 0);
	h_eq_i64("21s min", in.lit, -32768);
	in = fake_dec(0x80000115, 0);
	h_eq_i64("21h 16 bits min", in.lit, INT32_MIN);
	in = fake_dec(0x7fff0115, 0);
	h_eq_i64("21h 16 bits max", in.lit, 0x7fff0000);
	in = fake_dec(0x80000119, 0);
	h_eq_i64("21h 48 bits min", in.lit, INT64_MIN);
	in = fake_dec(0x7fff0119, 0);
	h_eq_i64("21h 48 bits max", in.lit, 0x7fff000000000000ll);
	in = fake_dec(0xffffff1a, 0);
	h_eq_i64("21c indice max", in.idx, 65535);
}

static void	decode_two_units_more(void)
{
	t_dinsn	in;

	in = fake_dec(0xfdfeff90, 0);
	h_eq_i64("23x AA", in.a, 255);
	h_eq_i64("23x BB", in.b, 254);
	h_eq_i64("23x CC", in.c, 253);
	in = fake_dec(0x80feffd8, 0);
	h_eq_i64("22b BB", in.b, 254);
	h_eq_i64("22b min", in.lit, -128);
	in = fake_dec(0x7ffeffd8, 0);
	h_eq_i64("22b max", in.lit, 127);
	in = fake_dec(0xffffef32, 0);
	h_eq_i64("22t A", in.a, 15);
	h_eq_i64("22t B", in.b, 14);
	h_eq_i64("22t negatif", in.lit, -1);
	in = fake_dec(0x7fffefd0, 0);
	h_eq_i64("22s max", in.lit, 32767);
	in = fake_dec(0x8000efd0, 0);
	h_eq_i64("22s min", in.lit, -32768);
	in = fake_dec(0xffffef52, 0);
	h_eq_i64("22c A", in.a, 15);
	h_eq_i64("22c B", in.b, 14);
	h_eq_i64("22c indice max", in.idx, 65535);
}

static void	decode_three_units(void)
{
	t_dinsn	in;

	in = fake_dec(0x80000000002aull, 0);
	h_eq_i64("30t min", in.lit, INT32_MIN);
	h_eq_i64("30t len", in.len, 3);
	in = fake_dec(0x7fffffff002aull, 0);
	h_eq_i64("30t max", in.lit, INT32_MAX);
	in = fake_dec(0xfffeffff0003ull, 0);
	h_eq_i64("32x AAAA", in.a, 65535);
	h_eq_i64("32x BBBB", in.b, 65534);
	in = fake_dec(0x7fffffffff14ull, 0);
	h_eq_i64("31i max", in.lit, INT32_MAX);
	in = fake_dec(0x80000000ff17ull, 0);
	h_eq_i64("31i min", in.lit, INT32_MIN);
	in = fake_dec(0xffffffffff2bull, 0);
	h_eq_i64("31t AA", in.a, 255);
	h_eq_i64("31t negatif", in.lit, -1);
	in = fake_dec(0xffffffffff1bull, 0);
	h_eq_u64("31c indice max", in.idx, 0xffffffffu);
}

int	main(void)
{
	h_begin("d04 decodage par format");
	h_run("formats 10x 12x 11n 11x 10t", decode_one_unit);
	h_run("formats 20t 22x 21t 21s 21h 21c", decode_two_units);
	h_run("formats 23x 22b 22t 22s 22c", decode_two_units_more);
	h_run("formats 30t 32x 31i 31t 31c", decode_three_units);
	return (h_end());
}
