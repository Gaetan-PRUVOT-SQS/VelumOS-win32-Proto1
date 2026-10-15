#include "fake.h"
#include "ctl_int.h"

t_rect	fake_mlevel(const t_ctl *m, int lvl)
{
	return (((const t_menu *)m->priv)->lv[lvl].rect);
}
