#include "help.h"

void	luna_metrics(t_lunametrics *out)
{
	static const t_lunametrics	m = {30, 4, 4, 21, 21, 30, 100, 80, 17, 22,
		16, 32};

	*out = m;
}

t_color	luna_color_window(void)
{
	return (0xffece9d8);
}
