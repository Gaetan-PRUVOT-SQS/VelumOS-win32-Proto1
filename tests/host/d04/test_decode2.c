#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"

static void	decode_calls(void)
{
	t_dinsn	in;

	in = fake_dec(0x4321ffff556eull, 0);
	h_eq_i64("35c compte", in.argc, 5);
	h_eq_i64("35c indice", in.idx, 65535);
	h_eq_i64("35c C", in.args[0], 1);
	h_eq_i64("35c D", in.args[1], 2);
	h_eq_i64("35c E", in.args[2], 3);
	h_eq_i64("35c F", in.args[3], 4);
	h_eq_i64("35c G", in.args[4], 5);
	h_eq_i64("35c compte 6 refuse", fake_dec_r(0x4321ffff656eull, 0), E_INVAL);
	in = fake_dec(0xffff1234ff77ull, 0);
	h_eq_i64("3rc compte max", in.argc, 255);
	h_eq_i64("3rc premier registre max", in.c, 65535);
	h_eq_i64("3rc indice", in.idx, 0x1234);
	in = fake_dec(0xbeef0010010225faull, 0);
	h_eq_i64("45cc len", in.len, 4);
	h_eq_i64("45cc compte", in.argc, 2);
	h_eq_i64("45cc D", in.args[1], 1);
	h_eq_i64("45cc indice", in.idx, 0x0102);
	h_eq_i64("45cc prototype", in.idx2, 0xbeef);
	in = fake_dec(0xcafe0009000703fbull, 0);
	h_eq_i64("4rcc compte", in.argc, 3);
	h_eq_i64("4rcc premier registre", in.c, 9);
	h_eq_i64("4rcc prototype", in.idx2, 0xcafe);
}

static void	decode_wide(void)
{
	t_dinsn	in;

	in = fake_dec(0xff18, 0x8000);
	h_eq_i64("51l len", in.len, 5);
	h_eq_i64("51l AA", in.a, 255);
	h_eq_i64("51l min", in.lit, INT64_MIN);
	in = fake_dec(0xffffffffffffff18ull, 0x7fff);
	h_eq_i64("51l max", in.lit, INT64_MAX);
	in = fake_dec(0x0807060504030218ull, 0x0a09);
	h_eq_i64("51l ordre des octets", in.lit, 0x0a09080706050403ll);
}

static int	short_by_one(int op)
{
	uint8_t	*buf;
	t_dinsn	in;
	t_span	s;
	int		r;

	s.len = 2 * (size_t)(dexcode_format_len(dexcode_format((uint8_t)op)) - 1);
	buf = malloc(s.len + 2);
	if (!buf)
		exit(2);
	memset(buf, 0, s.len + 2);
	buf[0] = (uint8_t)op;
	s.p = buf;
	r = dexcode_decode(s, 0, &in);
	s.len += 2;
	r = (r == E_INVAL && dexcode_decode(s, 0, &in) == 0);
	free(buf);
	return (r);
}

static void	decode_truncated(void)
{
	t_dinsn	in;
	t_span	s;
	int		op;
	int		bad;

	op = 0;
	bad = 0;
	while (op < 256)
	{
		if (dexcode_format((uint8_t)op) != DF_NONE)
			bad += !short_by_one(op);
		op++;
	}
	h_eq_i64("troncature d une unite refusee pour chaque code", bad, 0);
	h_eq_i64("code inutilise", fake_dec_r(0x003e, 0), E_INVAL);
	s.p = g_c.b;
	s.len = 10;
	h_eq_i64("pc a la fin", dexcode_decode(s, 5, &in), E_INVAL);
	h_eq_i64("sortie nulle", dexcode_decode(s, 0, NULL), E_INVAL);
	s.p = NULL;
	h_eq_i64("code nul", dexcode_decode(s, 0, &in), E_INVAL);
}

int	main(void)
{
	h_begin("d04 decodage des appels et troncature");
	h_run("formats 35c 3rc 45cc 4rcc", decode_calls);
	h_run("format 51l", decode_wide);
	h_run("troncature de chaque code", decode_truncated);
	return (h_end());
}
