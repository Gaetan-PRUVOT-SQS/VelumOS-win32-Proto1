#include "sys_int.h"
#include "string.h"

int64_t	sys_cstrlen(const char *s, size_t max)
{
	if (!s)
		return (E_FAULT);
	return ((int64_t)strnlen(s, max + 1));
}
