#include "dr_int.h"

t_droid	*dr_host(t_dvm *vm)
{
	return ((t_droid *)vm->host);
}

int	dr_npe(t_dvm *vm)
{
	return (dvm_throw(vm, DR_NPE, NULL));
}

t_dclass	*dr_class(t_dvm *vm, const char *desc)
{
	t_dclass	*c;

	c = NULL;
	if (!desc || dvm_class(vm, desc, &c) != 0)
		return (NULL);
	return (c);
}

int	dr_is(t_dvm *vm, t_dref ref, const char *desc)
{
	return (dvm_is_instance(vm, ref, dr_class(vm, desc)));
}

size_t	dr_dotted(const char *desc, char *out, size_t cap)
{
	size_t	n;
	size_t	i;

	n = 0;
	i = 0;
	if (!desc || cap == 0)
		return (0);
	if (desc[0] == 'L')
		i = 1;
	while (desc[i] && n + 1 < cap && !(desc[0] == 'L' && desc[i] == ';'))
	{
		out[n] = desc[i];
		if (desc[i] == '/')
			out[n] = '.';
		n++;
		i++;
	}
	out[n] = '\0';
	return (n);
}
