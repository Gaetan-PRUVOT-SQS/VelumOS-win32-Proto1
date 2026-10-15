#include <string.h>
#include "init.h"
#include "velum/velum.h"

int	service_start(t_service *s)
{
	int64_t	h;

	h = v_spawn(s->path, NULL, 0, s->flags);
	if (h < 0)
	{
		v_logf(V_LOG_ERR, "init: lancement de %s impossible (%lld)",
			s->path, (long long)h);
		if (h == E_NOENT)
			s->enabled = 0;
		return ((int)h);
	}
	s->h = (t_handle)h;
	v_logf(V_LOG_INFO, "init: %s lancé", s->path);
	return (0);
}

static int64_t	service_exit_code(t_handle h)
{
	t_procinfo	info;
	int			rc;

	memset(&info, 0, sizeof(info));
	rc = v_proc_info(h, &info);
	if (rc < 0)
		return (rc);
	return ((int32_t)info.reserved);
}

void	service_died(t_service *s, uint64_t now_ns)
{
	v_logf(V_LOG_WARN, "init: %s terminé (code %lld)", s->path,
		(long long)service_exit_code(s->h));
	v_close(s->h);
	s->h = 0;
	if (!restart_allowed(&s->rs, now_ns))
	{
		v_logf(V_LOG_ERR, "init: %s relancé %d fois en 30 s, abandon",
			s->path, RESTART_MAX);
		s->enabled = 0;
		return ;
	}
	if (service_start(s) < 0)
		s->enabled = 0;
}
