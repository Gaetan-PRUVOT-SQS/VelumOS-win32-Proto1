#include <stdlib.h>
#include <string.h>
#include "d00.h"

int	d00_same(const char *a, const char *b)
{
	return (strcmp(a, b) == 0);
}

static int	verify_code(const t_dex *d, uint32_t code_off)
{
	t_dexcode		c;
	t_dcodelimits	lim;
	uint8_t			*starts;
	int				rc;

	if (dex_code(d, code_off, &c) < 0)
		return (-1);
	lim = (t_dcodelimits){c.registers, c.ins, c.outs, d->n[DEX_T_STRING],
		d->n[DEX_T_TYPE], d->n[DEX_T_FIELD], d->n[DEX_T_METHOD],
		d->n[DEX_T_PROTO]};
	starts = calloc(1, c.insns_size / 8 + 1);
	if (!starts)
		return (-1);
	rc = dexcode_verify((t_span){c.insns, (size_t)c.insns_size * 2}, &lim,
			starts);
	free(starts);
	return (rc);
}

static uint32_t	verify_class(const t_dex *d, uint32_t def, uint32_t *refused)
{
	t_dexclass	c;
	t_dexcdata	it;
	t_dexmember	m;
	uint32_t	done;

	done = 0;
	if (dex_class(d, def, &c) < 0 || dex_cdata_open(d, &c, &it) < 0)
		return (0);
	while (dex_cdata_next(&it, &m) == 0)
	{
		if (m.kind < DEX_M_DIRECT || m.code_off == 0)
			continue ;
		done++;
		if (verify_code(d, m.code_off) != 0)
			(*refused)++;
	}
	return (done);
}

uint32_t	d00_verify_all(const t_dex *d, uint32_t *refused)
{
	uint32_t	def;
	uint32_t	done;

	def = 0;
	done = 0;
	*refused = 0;
	while (def < d->n[DEX_T_CLASS])
	{
		done += verify_class(d, def, refused);
		def++;
	}
	return (done);
}

int	d00_attr_named(const t_axml *x, const char *name, t_axmlattr *out)
{
	char		text[D00_TEXT];
	uint32_t	i;

	i = 0;
	while (i < axml_attr_count(x))
	{
		if (axml_attr(x, i, out) == 0 && axml_string(x, out->name,
				(t_text){text, sizeof(text)}) >= 0 && d00_same(text, name))
			return (0);
		i++;
	}
	return (-1);
}
