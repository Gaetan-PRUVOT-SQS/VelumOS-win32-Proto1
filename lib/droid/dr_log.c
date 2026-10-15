#include "dr_int.h"

static int	dr_log(t_dvm *vm, const uint32_t *args, uint64_t *ret, uint32_t lv)
{
	char	tag[DROID_TEXT_MAX];
	char	msg[DROID_TEXT_MAX];
	t_droid	*d;
	int		rc;

	d = dr_host(vm);
	rc = dr_cstr(vm, args[0], tag);
	if (rc == 0)
		rc = dr_cstr(vm, args[1], msg);
	if (rc != 0)
		return (rc);
	if (d && d->log)
		d->log(d->user, lv, tag, msg);
	*ret = (uint32_t)strlen(msg);
	return (0);
}

int	dr_log_d(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	return (dr_log(vm, args, ret, DROID_LOG_DEBUG));
}

int	dr_log_i(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	return (dr_log(vm, args, ret, DROID_LOG_INFO));
}

int	dr_log_w(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	return (dr_log(vm, args, ret, DROID_LOG_WARN));
}

int	dr_log_e(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	return (dr_log(vm, args, ret, DROID_LOG_ERROR));
}
