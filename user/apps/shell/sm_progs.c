#include "velum/err.h"
#include "velum/libk.h"
#include "sm_progs.h"

static int	add_line(t_smprogs *out, const char *s, size_t n)
{
	if (out->count >= SM_PROGS_MAX
		|| sm_progs_line(&out->list[out->count], s, n) < 0)
		return (E_INVAL);
	out->count++;
	return (0);
}

int	sm_progs_parse(const char *buf, size_t len, t_smprogs *out)
{
	size_t	start;
	size_t	i;
	int		r;

	memset(out, 0, sizeof(*out));
	if (!buf || len == 0 || len > SM_PROGS_FILE_MAX)
		return (E_INVAL);
	start = 0;
	i = 0;
	r = 0;
	while (i <= len && r == 0)
	{
		if (i == len || buf[i] == '\n')
		{
			if (i > start || i < len)
				r = add_line(out, buf + start, i - start);
			start = i + 1;
		}
		i++;
	}
	if (r < 0)
		memset(out, 0, sizeof(*out));
	if (out->count == 0)
		return (E_INVAL);
	return ((int)out->count);
}

void	sm_progs_default(t_smprogs *out)
{
	memset(out, 0, sizeof(*out));
	strlcpy(out->list[0].name, SM_PROG_DEFAULT_NAME, SM_PROG_NAME_MAX);
	strlcpy(out->list[0].path, SM_PROG_DEFAULT_PATH, SM_PROG_PATH_MAX);
	out->count = 1;
}
