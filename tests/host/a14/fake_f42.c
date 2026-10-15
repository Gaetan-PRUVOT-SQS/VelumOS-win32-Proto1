#include <string.h>
#include "fake_f4.h"
#include "fake_sys.h"

const uint64_t	*f4_open(const t_f4_stack *s, size_t nwords)
{
	uint64_t	*buf;

	buf = fake_guarded(nwords * sizeof(uint64_t));
	if (buf && nwords)
		memcpy(buf, s->w, nwords * sizeof(uint64_t));
	return (buf);
}

void	f4_close(const uint64_t *buf, size_t nwords)
{
	fake_guarded_free((void *)buf, nwords * sizeof(uint64_t));
}

int	f4_parse(const t_f4_stack *s, size_t nwords, t_startinfo *info)
{
	const uint64_t	*buf;
	int				rc;

	buf = f4_open(s, nwords);
	rc = start_parse(buf, nwords, info);
	f4_close(buf, nwords);
	return (rc);
}
