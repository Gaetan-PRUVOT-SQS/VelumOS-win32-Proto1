#include "heap_int.h"
#include "velum/panic.h"

_Noreturn void	heap_fault(const char *what, const void *p, const void *at)
{
	panic("heap: %s %p (appelant %p)", what, p, at);
}
