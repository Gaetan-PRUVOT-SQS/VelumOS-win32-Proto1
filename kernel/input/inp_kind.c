#include "inp_queue.h"
#include "velum/kinput.h"

uint32_t	inp_kind_of(uint32_t type)
{
	if (type == INP_KEY_DOWN || type == INP_KEY_UP || type == INP_CHAR)
		return (INPUT_KIND_KEYBOARD);
	if (type >= INP_MOUSE_MOVE && type <= INP_WHEEL)
		return (INPUT_KIND_MOUSE);
	return (INPUT_KIND_ALL);
}

int	inp_kind_match(uint32_t kind, uint32_t type)
{
	return (kind == INPUT_KIND_ALL || kind == inp_kind_of(type));
}

void	inpq_push_all(const t_inpevent *ev, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		input_push(&ev[i]);
		i++;
	}
}
