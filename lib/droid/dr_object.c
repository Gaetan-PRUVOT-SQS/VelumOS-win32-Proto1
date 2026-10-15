#include "dr_int.h"

int	dr_nop(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	(void)vm;
	(void)args;
	(void)ret;
	return (0);
}

static size_t	dr_hex(uint32_t v, char *out)
{
	size_t	n;
	int		sh;

	n = 0;
	sh = 28;
	while (sh > 0 && ((v >> sh) & 15) == 0)
		sh -= 4;
	while (sh >= 0)
	{
		out[n++] = "0123456789abcdef"[(v >> sh) & 15];
		sh -= 4;
	}
	out[n] = '\0';
	return (n);
}

int	dr_obj_tostring(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	char		buf[DROID_TEXT_MAX + 16];
	const char	*desc;
	size_t		n;

	desc = dvm_class_name(dvm_class_of(vm, args[0]));
	if (!desc)
		return (dr_npe(vm));
	n = dr_dotted(desc, buf, DROID_TEXT_MAX);
	buf[n++] = '@';
	dr_hex(args[0], buf + n);
	return (dr_new8(vm, buf, ret));
}

int	dr_obj_equals(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	(void)vm;
	*ret = (args[0] == args[1]);
	return (0);
}

int	dr_obj_hash(t_dvm *vm, const uint32_t *args, uint64_t *ret)
{
	(void)vm;
	*ret = args[0];
	return (0);
}
