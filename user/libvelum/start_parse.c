#include "string.h"
#include "velum/err.h"
#include "velum/vstart.h"

static void	start_aux_store(t_startinfo *o, uint64_t type, uint64_t val)
{
	if (type == AT_PAGESZ)
		o->pagesz = val;
	else if (type == AT_RANDOM)
		o->random = (const uint8_t *)(uintptr_t)val;
	else if (type == AT_PHDR)
		o->phdr = (const void *)(uintptr_t)val;
	else if (type == AT_PHNUM)
		o->phnum = val;
	else if (type == AT_ENTRY)
		o->entry = val;
	else if (type == AT_VELUM_ABI)
		o->abi = val;
	else if (type == AT_VELUM_FLAGS)
		o->flags = val;
}

static int	start_aux_scan(const uint64_t *w, size_t from, size_t nwords,
		t_startinfo *o)
{
	size_t	pairs;

	pairs = 0;
	while (from < nwords && pairs < START_AUX_MAX)
	{
		if (w[from] == AT_NULL)
			return (START_OK);
		if (from + 1 >= nwords)
			return (START_TRUNCATED);
		start_aux_store(o, w[from], w[from + 1]);
		from += 2;
		pairs++;
	}
	return (START_TRUNCATED);
}

static int	start_env_scan(const uint64_t *w, size_t from, size_t nwords,
		t_startinfo *o)
{
	size_t	n;

	n = 0;
	while (from + n < nwords && w[from + n])
	{
		if (n >= START_ENVC_MAX)
			return (E_INVAL);
		n++;
	}
	if (from + n >= nwords)
		return (E_PROTO);
	o->envp = (char **)(uintptr_t)(w + from);
	o->envc = n;
	return ((int)(from + n + 1));
}

static int	start_argv_check(const uint64_t *w, size_t nwords,
		t_startinfo *o)
{
	uint64_t	i;

	if (w[0] > START_ARGC_MAX)
		return (E_RANGE);
	if (w[0] + 2 > nwords)
		return (E_PROTO);
	i = 0;
	while (i < w[0])
	{
		if (!w[1 + i])
			return (E_PROTO);
		i++;
	}
	if (w[1 + w[0]])
		return (E_PROTO);
	o->argc = w[0];
	o->argv = (char **)(uintptr_t)(w + 1);
	return (0);
}

int	start_parse(const uint64_t *sp, size_t nwords, t_startinfo *out)
{
	int	rc;

	if (!sp || !out || nwords < 3)
		return (E_INVAL);
	memset(out, 0, sizeof(*out));
	rc = start_argv_check(sp, nwords, out);
	if (rc < 0)
		return (rc);
	rc = start_env_scan(sp, 2 + out->argc, nwords, out);
	if (rc < 0)
		return (rc);
	return (start_aux_scan(sp, (size_t)rc, nwords, out));
}
