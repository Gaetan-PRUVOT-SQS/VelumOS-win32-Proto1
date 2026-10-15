#include <string.h>
#include "d09.h"

uint64_t	fk_dbits(double d)
{
	uint64_t	bits;

	memcpy(&bits, &d, sizeof(bits));
	return (bits);
}

uint32_t	fk_fbits(float f)
{
	uint32_t	bits;

	memcpy(&bits, &f, sizeof(bits));
	return (bits);
}

int	fk_same(const char *a, const char *b)
{
	return (a && b && strcmp(a, b) == 0);
}

const char	*fk_pending(void)
{
	if (g_fk.vm.pending == DVM_NULL || g_fk.vm.pending >= FK_OBJS)
		return ("aucune");
	return (g_fk.obj[g_fk.vm.pending].desc);
}

const char	*fk_type(uint32_t idx)
{
	t_dexstr	s;

	if (dex_type(&g_fk.dex, idx, &s) < 0)
		return ("?");
	return (s.p);
}
