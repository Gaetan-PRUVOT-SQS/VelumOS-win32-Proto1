#include "smp_int.h"
#include "velum/err.h"

#define XAPIC_ID_MAX 0xfe

int	smp_parse_count(const char *text, uint32_t *out)
{
	uint64_t	value;

	if (!text || !out || !*text)
		return (E_INVAL);
	value = 0;
	while (*text)
	{
		if (*text < '0' || *text > '9')
			return (E_INVAL);
		value = value * 10 + (uint64_t)(*text - '0');
		if (value > CPU_MAX)
			value = CPU_MAX;
		text++;
	}
	if (value == 0)
		return (E_INVAL);
	*out = (uint32_t)value;
	return (E_OK);
}

static bool	plan_known(const uint32_t *list, uint32_t n, uint32_t id)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		if (list[i] == id)
			return (true);
		i++;
	}
	return (false);
}

static bool	plan_usable(const t_plan_in *in, const uint32_t *out, uint32_t n,
	uint32_t id)
{
	if (id == in->bsp)
		return (false);
	if (!in->x2 && id > XAPIC_ID_MAX)
		return (false);
	return (!plan_known(out, n, id));
}

uint32_t	smp_plan(const t_plan_in *in, uint32_t *ap_out, uint32_t cap)
{
	uint32_t	n;
	uint32_t	i;

	n = 0;
	i = 0;
	if (!in || !ap_out || !in->ids)
		return (0);
	while (i < in->count && n + 1 < in->want && n < cap)
	{
		if (plan_usable(in, ap_out, n, in->ids[i]))
			ap_out[n++] = in->ids[i];
		i++;
	}
	return (n);
}
