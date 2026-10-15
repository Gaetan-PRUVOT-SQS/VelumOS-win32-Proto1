#include "in_int.h"

uint32_t	in_get(t_in *in, uint32_t n)
{
	if (n >= in->nreg)
	{
		in->bad = 1;
		return (0);
	}
	return (in->r[n]);
}

void	in_set(t_in *in, uint32_t n, uint32_t v)
{
	if (n >= in->nreg)
		in->bad = 1;
	else
		in->r[n] = v;
}

uint64_t	in_getw(t_in *in, uint32_t n)
{
	if (n >= in->nreg || in->nreg - n < 2)
	{
		in->bad = 1;
		return (0);
	}
	return (in->r[n] | ((uint64_t)in->r[n + 1] << 32));
}

void	in_setw(t_in *in, uint32_t n, uint64_t v)
{
	if (n >= in->nreg || in->nreg - n < 2)
	{
		in->bad = 1;
		return ;
	}
	in->r[n] = (uint32_t)v;
	in->r[n + 1] = (uint32_t)(v >> 32);
}

int	in_throw(t_in *in, const char *desc)
{
	int	rc;

	rc = dvm_throw(in->vm, desc, NULL);
	if (rc == 0)
		return (DVM_THROWN);
	return (rc);
}
