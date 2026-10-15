#include "luna_ctrl.h"

bool	lp_btn_primary(const t_lunabtn *b)
{
	if (b->state == LS_DISABLED)
		return (false);
	return (b->is_default || b->state == LS_DEFAULT);
}

t_color	luna_button_ink(const t_lunabtn *b)
{
	t_cpal	pal;

	pal = lp_pal();
	if (b == NULL)
		return (pal->ink);
	if (b->state == LS_DISABLED)
		return (pal->dim);
	if (lp_btn_primary(b))
		return (LC_WHITE);
	return (pal->ink);
}
