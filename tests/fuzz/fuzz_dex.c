#include <stdlib.h>
#include <string.h>
#include "dex_int.h"
#include "fake.h"
#include "fuzz.h"
#include "velum/apk/dexcode.h"

#define DEX_FUZZ_MAX 0x10000u
#define DEX_FUZZ_CODE_MAX 64u

static void	check_code(const t_dex *d, const t_dexcode *code)
{
	t_dcodelimits	lim;
	t_dinsn			in;
	t_span			insns;
	uint8_t			*starts;
	uint32_t		pc;

	if (code->insns_size == 0 || code->insns_size > 0x4000)
		return ;
	lim.registers = code->registers;
	lim.ins = code->ins;
	lim.outs = code->outs;
	lim.strings = d->n[DEX_T_STRING];
	lim.types = d->n[DEX_T_TYPE];
	lim.fields = d->n[DEX_T_FIELD];
	lim.methods = d->n[DEX_T_METHOD];
	lim.protos = d->n[DEX_T_PROTO];
	insns.p = code->insns;
	insns.len = 2 * (size_t)code->insns_size;
	starts = malloc((code->insns_size + 7) / 8);
	if (starts == NULL)
		abort();
	if (dexcode_verify(insns, &lim, starts) == 0)
	{
		pc = 0;
		while (pc < code->insns_size)
		{
			if ((starts[pc >> 3] >> (pc & 7)) & 1)
				FUZZ_REQUIRE(dexcode_decode(insns, pc, &in) == 0
					&& pc + in.len <= code->insns_size);
			pc++;
		}
	}
	free(starts);
}

static void	check_class_code(const t_dex *d, uint32_t index, uint32_t *budget)
{
	t_dexclass	c;
	t_dexcdata	it;
	t_dexmember	m;
	t_dexcode	code;

	if (dex_class(d, index, &c) != E_OK || dex_cdata_open(d, &c, &it) != E_OK)
		return ;
	while (*budget > 0 && dex_cdata_next(&it, &m) == E_OK)
	{
		if (m.code_off != 0 && dex_code(d, m.code_off, &code) == E_OK)
		{
			check_code(d, &code);
			(*budget)--;
		}
	}
}

static void	run_dex(const uint8_t *p, size_t len)
{
	t_dex		d;
	t_span		s;
	uint32_t	i;
	uint32_t	budget;

	s.p = p;
	s.len = len;
	if (dex_open(&d, s) != E_OK)
		return ;
	fake_walk(&d);
	budget = DEX_FUZZ_CODE_MAX;
	i = 0;
	while (i < d.n[DEX_T_CLASS] && budget > 0)
		check_class_code(&d, i++, &budget);
}

static void	put_u32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

static void	seal(uint8_t *p, size_t n)
{
	put_u32(p + 32, (uint32_t)n);
	put_u32(p + 8, dex_adler32(p + 12, n - 12));
}

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	uint8_t	*copy;

	if (size > DEX_FUZZ_MAX)
		return (0);
	run_dex(data, size);
	if (size < 0x70)
		return (0);
	copy = malloc(size);
	if (copy == NULL)
		abort();
	memcpy(copy, data, size);
	seal(copy, size);
	run_dex(copy, size);
	free(copy);
	return (0);
}
