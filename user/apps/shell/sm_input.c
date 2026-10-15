#include "velum/abi/abi_input.h"
#include "startmenu.h"

int	sm_activate(t_startmenu *m, int32_t idx)
{
	const t_smitem	*it;

	if (!m->open || idx < 0 || !sm_visible((uint32_t)idx))
		return (SMC_NONE);
	it = sm_item((uint32_t)idx);
	sm_close(m);
	return ((int)it->cmd);
}

static int	key_enter(t_startmenu *m)
{
	if (m->focus >= 0)
		return (sm_activate(m, m->focus));
	return (sm_activate(m, m->hot));
}

static void	key_edge(t_startmenu *m, int32_t idx)
{
	m->focus = idx;
	m->hot = SM_NO_ITEM;
}

int	sm_key(t_startmenu *m, uint32_t vk)
{
	if (!m->open)
		return (SMC_NONE);
	if (vk == VK_DOWN || vk == VK_RIGHT || vk == VK_TAB)
		sm_move(m, 1);
	else if (vk == VK_UP || vk == VK_LEFT)
		sm_move(m, -1);
	else if (vk == VK_HOME)
		key_edge(m, SM_IDX_RUN);
	else if (vk == VK_END)
		key_edge(m, SM_IDX_SHUTDOWN);
	else if (vk == VK_RETURN || vk == VK_SPACE)
		return (key_enter(m));
	else if (vk == VK_ESCAPE || vk == VK_LWIN || vk == VK_RWIN)
		sm_close(m);
	return (SMC_NONE);
}
