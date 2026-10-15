#include "stdlib.h"
#include "string.h"
#include "velum/err.h"
#include "velum/vproc.h"

static int64_t	args_total(const char *const *argv)
{
	int64_t	total;
	size_t	n;

	if (!argv)
		return (E_FAULT);
	total = 0;
	n = 0;
	while (argv[n])
	{
		if (n >= V_ARGC_MAX)
			return (E_INVAL);
		total += (int64_t)strnlen(argv[n], V_ARGS_MAX + 1) + 1;
		if (total > V_ARGS_MAX)
			return (E_INVAL);
		n++;
	}
	return (total);
}

int64_t	v_args_pack(char *buf, size_t cap, const char *const *argv)
{
	int64_t	total;
	size_t	off;
	size_t	len;
	size_t	i;

	total = args_total(argv);
	if (total < 0 || (size_t)total > cap)
		return (total);
	off = 0;
	i = 0;
	while (argv[i])
	{
		len = strlen(argv[i]) + 1;
		memcpy(buf + off, argv[i], len);
		off += len;
		i++;
	}
	return (total);
}

static const char	*const	*spawn_rest(const char *const *argv)
{
	if (argv && argv[0])
		return (argv + 1);
	return (argv);
}

int64_t	v_spawnv(const char *path, const char *const *argv, uint32_t flags)
{
	int64_t	total;
	int64_t	rc;
	char	*blob;

	argv = spawn_rest(argv);
	total = v_args_pack(NULL, 0, argv);
	if (total < 0)
		return (total);
	blob = malloc((size_t)total + 1);
	if (!blob)
		return (E_NOMEM);
	v_args_pack(blob, (size_t)total + 1, argv);
	rc = v_spawn(path, blob, (size_t)total, flags);
	free(blob);
	return (rc);
}
