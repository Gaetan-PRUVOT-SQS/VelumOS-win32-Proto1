#include "../common/layout.h"
#include "startmenu.h"

t_rect	sm_item_rect(const t_smlayout *l, int32_t idx)
{
	if (!l || idx < 0)
		return (lay_rect(0, 0, 0, 0));
	if (idx < l->ntiles && idx < LUNA_LAUNCHER_TILES)
		return (l->tile[idx]);
	if (idx == SM_IDX_LOGOFF)
		return (l->logoff);
	if (idx == SM_IDX_SHUTDOWN)
		return (l->poweroff);
	return (lay_rect(0, 0, 0, 0));
}
