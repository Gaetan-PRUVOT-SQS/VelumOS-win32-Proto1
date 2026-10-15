#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"

static const uint8_t	g_mask[DF_COUNT] = {0, 0, 3, 1, 1, 0, 0, 3, 1, 1, 1, 1,
	7, 3, 3, 3, 3, 0, 3, 1, 1, 1, 0, 0, 0, 0, 1};

t_dcodelimits	fake_lim(uint32_t regs)
{
	t_dcodelimits	l;

	l.registers = regs;
	l.ins = 0;
	l.outs = 2;
	l.strings = 4;
	l.types = 4;
	l.fields = 4;
	l.methods = 4;
	l.protos = 4;
	return (l);
}

static int	insn_bad(const t_dinsn *in, uint32_t nreg)
{
	uint8_t	m;
	int		bad;
	int		i;

	m = g_mask[in->fmt];
	bad = ((m & 1) && in->a >= nreg) || ((m & 2) && in->b >= nreg)
		|| ((m & 4) && in->c >= nreg);
	i = 0;
	while ((in->fmt == DF_35C || in->fmt == DF_45CC) && i < in->argc)
		bad += (in->args[i++] >= nreg);
	if ((in->fmt == DF_3RC || in->fmt == DF_4RCC) && in->argc > 0)
		bad += ((uint64_t)in->c + in->argc > nreg);
	return (bad);
}

int	fake_property(t_span s, const t_dcodelimits *lim, const uint8_t *st)
{
	t_dinsn		in;
	uint32_t	pc;
	int			bad;

	pc = 0;
	bad = 0;
	while (pc < s.len / 2)
	{
		if ((st[pc >> 3] >> (pc & 7)) & 1)
		{
			if (dexcode_decode(s, pc, &in) < 0 || pc + in.len > s.len / 2)
				bad++;
			else
				bad += insn_bad(&in, lim->registers);
		}
		pc++;
	}
	return (bad);
}

int	fake_exact(const uint8_t *src, uint32_t units, const t_dcodelimits *lim)
{
	t_span	s;
	uint8_t	*st;
	uint8_t	*code;
	int		r;

	code = malloc(units * 2 + (units == 0));
	st = malloc((units + 7) / 8 + (units == 0));
	if (!code || !st)
		exit(2);
	memcpy(code, src, units * 2);
	s.p = code;
	s.len = units * 2;
	r = dexcode_verify(s, lim, st);
	if (r == 0)
		h_true(fake_property(s, lim, st) == 0, "propriete apres acceptation");
	memset(g_c.starts, 0, sizeof(g_c.starts));
	if (r == 0 && (units + 7) / 8 <= sizeof(g_c.starts))
		memcpy(g_c.starts, st, (units + 7) / 8);
	g_c.accepted += (r == 0);
	free(code);
	free(st);
	return (r);
}

int	fake_v(uint32_t regs)
{
	t_dcodelimits	l;

	l = fake_lim(regs);
	return (fake_exact(g_c.b, g_c.n, &l));
}
