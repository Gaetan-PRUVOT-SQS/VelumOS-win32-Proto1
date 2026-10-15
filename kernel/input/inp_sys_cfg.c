#include "inp_sys.h"
#include "velum/err.h"
#include "velum/kinput.h"
#include "velum/libk.h"
#include "velum/vmm.h"

static int64_t	layout_get(t_uptr name, uint64_t cap)
{
	const char	*cur;
	size_t		len;
	int			rc;

	cur = input_layout();
	len = strlen(cur);
	if (cap < len + 1)
		return (E_RANGE);
	rc = copy_to_user(name, cur, len + 1);
	if (rc < 0)
		return (rc);
	return ((int64_t)len);
}

static int64_t	layout_set(t_uptr name, uint64_t len)
{
	char	buf[INPUT_LAYOUT_NAME_MAX + 1];
	int		rc;

	if (len == 0 || len > INPUT_LAYOUT_NAME_MAX)
		return (E_INVAL);
	rc = copy_from_user(buf, name, len);
	if (rc < 0)
		return (rc);
	buf[len] = '\0';
	if (memchr(buf, 0, len))
		return (E_INVAL);
	return (input_set_layout(buf));
}

int64_t	sys_input_layout(const t_sysargs *args)
{
	if (args->a[0] == INPUT_LAYOUT_GET)
		return (layout_get(args->a[1], args->a[2]));
	if (args->a[0] != INPUT_LAYOUT_SET)
		return (E_INVAL);
	if (!inp_sys_may_configure())
		return (E_PERM);
	return (layout_set(args->a[1], args->a[2]));
}

int64_t	sys_input_leds(const t_sysargs *args)
{
	if (!inp_sys_may_configure())
		return (E_PERM);
	if (args->a[0] > INPUT_LED_ALL)
		return (E_INVAL);
	input_set_leds((uint32_t)args->a[0]);
	return (0);
}

int64_t	sys_input_mouse_cfg(const t_sysargs *args)
{
	if (!inp_sys_may_configure())
		return (E_PERM);
	if (args->a[0] < INPUT_MOUSE_SPEED_MIN)
		return (E_INVAL);
	if (args->a[0] > INPUT_MOUSE_SPEED_MAX || args->a[1] > 1)
		return (E_INVAL);
	return (input_mouse_config((uint32_t)args->a[0], (uint32_t)args->a[1]));
}
