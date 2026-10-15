#include "velum/libk.h"
#include "../common/platform.h"
#include "logon.h"

int	logon_load(t_logon *lg)
{
	static char	buf[ACC_FILE_MAX + 1];
	size_t		len;
	int			r;
	uint32_t	i;

	lg->nshown = 0;
	r = os_read_file(ACC_USERS_PATH, buf, sizeof(buf), &len);
	if (r == 0)
		r = acc_parse_file(buf, len, &lg->set);
	secure_zero(buf, sizeof(buf));
	if (r < 0)
		return (r);
	i = 0;
	while (i < lg->set.count && lg->nshown < LOGON_TILES_MAX)
	{
		if (!(lg->set.list[i].flags & ACC_FLAG_DISABLED))
		{
			lg->shown[lg->nshown] = i;
			lg->nshown++;
		}
		i++;
	}
	return ((int)lg->nshown);
}

uint32_t	logon_remaining(const t_logon *lg)
{
	if (lg->flow.selected >= lg->nshown)
		return (0);
	return (lim_remaining_s(&lg->lim, lg->shown[lg->flow.selected],
			os_mono_ns()));
}
