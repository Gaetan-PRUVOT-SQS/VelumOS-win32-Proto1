#include "velum/err.h"
#include "velum/libk.h"
#include "accounts.h"

const t_account	*acc_find(const t_accounts *set, const char *name)
{
	uint32_t	i;

	i = 0;
	while (i < set->count)
	{
		if (strcmp(set->list[i].name, name) == 0)
			return (&set->list[i]);
		i++;
	}
	return (NULL);
}

static size_t	next_line(const char *text, size_t len, size_t *pos)
{
	const char	*start;
	const char	*nl;
	size_t		n;

	start = text + *pos;
	nl = memchr(start, '\n', len - *pos);
	n = len - *pos;
	if (nl)
		n = (size_t)(nl - start);
	*pos += n + 1;
	if (n && start[n - 1] == '\r')
		n--;
	return (n);
}

int	acc_parse_file(const char *text, size_t len, t_accounts *set)
{
	t_account	a;
	size_t		pos;
	size_t		start;
	size_t		n;

	if (!set || (!text && len))
		return (E_INVAL);
	memset(set, 0, sizeof(*set));
	if (len > ACC_FILE_MAX)
		return (E_RANGE);
	pos = 0;
	while (pos < len && set->count < ACC_MAX)
	{
		start = pos;
		n = next_line(text, len, &pos);
		if (acc_parse_line(text + start, n, &a) == 0
			&& !acc_find(set, a.name))
		{
			set->list[set->count] = a;
			set->count++;
		}
	}
	return ((int)set->count);
}
