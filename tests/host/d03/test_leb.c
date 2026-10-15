#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static uint32_t	uleb_of(const char *bytes, size_t n, int *err)
{
	t_dexcur	c;
	uint32_t	v;

	c.p = (const uint8_t *)bytes;
	c.len = n;
	c.pos = 0;
	c.err = 0;
	v = dex_uleb(&c);
	*err = c.err;
	return (v);
}

static int32_t	sleb_of(const char *bytes, size_t n, int *err)
{
	t_dexcur	c;
	int32_t		v;

	c.p = (const uint8_t *)bytes;
	c.len = n;
	c.pos = 0;
	c.err = 0;
	v = dex_sleb(&c);
	*err = c.err;
	return (v);
}

static void	uleb_limits(void)
{
	int	e;

	h_eq_u64("0", uleb_of("\x00", 1, &e), 0);
	h_eq_u64("127", uleb_of("\x7f", 1, &e), 127);
	h_eq_u64("128", uleb_of("\x80\x01", 2, &e), 128);
	h_eq_u64("16383", uleb_of("\xff\x7f", 2, &e), 16383);
	h_eq_u64("16384", uleb_of("\x80\x80\x01", 3, &e), 16384);
	h_eq_u64("2^28-1", uleb_of("\xff\xff\xff\x7f", 4, &e), 0x0fffffff);
	h_eq_u64("2^28", uleb_of("\x80\x80\x80\x80\x01", 5, &e), 0x10000000);
	h_eq_u64("2^32-1", uleb_of("\xff\xff\xff\xff\x0f", 5, &e), 0xffffffffu);
	h_eq_i64("accepte", e, 0);
	uleb_of("\xff\xff\xff\xff\x10", 5, &e);
	h_eq_i64("33 bits refuse", e, 1);
	uleb_of("\x80\x80\x80\x80\x80\x01", 6, &e);
	h_eq_i64("6 octets refuse", e, 1);
	uleb_of("\x80\x80", 2, &e);
	h_eq_i64("tronque refuse", e, 1);
	uleb_of("", 0, &e);
	h_eq_i64("vide refuse", e, 1);
}

static void	sleb_limits(void)
{
	int	e;

	h_eq_i64("0", sleb_of("\x00", 1, &e), 0);
	h_eq_i64("-1", sleb_of("\x7f", 1, &e), -1);
	h_eq_i64("63", sleb_of("\x3f", 1, &e), 63);
	h_eq_i64("-64", sleb_of("\x40", 1, &e), -64);
	h_eq_i64("64", sleb_of("\xc0\x00", 2, &e), 64);
	h_eq_i64("-65", sleb_of("\xbf\x7f", 2, &e), -65);
	h_eq_i64("max", sleb_of("\xff\xff\xff\xff\x07", 5, &e), INT32_MAX);
	h_eq_i64("min", sleb_of("\x80\x80\x80\x80\x78", 5, &e), INT32_MIN);
	h_eq_i64("accepte", e, 0);
	sleb_of("\x80\x80\x80\x80\x80\x00", 6, &e);
	h_eq_i64("6 octets refuse", e, 1);
	sleb_of("\xff\xff", 2, &e);
	h_eq_i64("tronque refuse", e, 1);
	sleb_of("", 0, &e);
	h_eq_i64("vide refuse", e, 1);
}

int	main(void)
{
	h_begin("d03/leb");
	h_run("ULEB128 : valeurs limites et refus", uleb_limits);
	h_run("SLEB128 : valeurs limites et refus", sleb_limits);
	return (h_end());
}
