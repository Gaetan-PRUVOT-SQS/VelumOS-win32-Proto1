#include "velum/err.h"
#include "velum/libk.h"
#include "velum/luna.h"
#include "startmenu.h"

_Static_assert(SM_IDX_LOGOFF == LUNA_LAUNCHER_LOGOFF
	&& SM_IDX_SHUTDOWN == LUNA_LAUNCHER_POWER, "indices du lanceur");
_Static_assert(1 + SM_PROGS_MAX <= LUNA_LAUNCHER_TILES, "tuiles du lanceur");

static t_smitem		g_items[SM_ITEMS] = {
{SMC_RUN, ICON_RUN, "Exécuter..."},
{SMC_LAUNCH_HELLO, ICON_PROGRAM, SM_PROG_DEFAULT_NAME},
{SMC_LAUNCH_B, ICON_PROGRAM, ""},
{SMC_LAUNCH_C, ICON_PROGRAM, ""},
{SMC_LOGOFF, ICON_USER, "Fermer la session"},
{SMC_SHUTDOWN, ICON_POWER, "Éteindre l'ordinateur"}
};
static uint32_t		g_nprogs = 1;

const t_smitem	*sm_item(uint32_t index)
{
	if (index >= SM_ITEMS)
		return (NULL);
	return (&g_items[index]);
}

uint32_t	sm_tiles(void)
{
	return (1 + g_nprogs);
}

int	sm_visible(uint32_t index)
{
	if (index >= SM_ITEMS)
		return (0);
	return (index < sm_tiles() || index >= SM_IDX_LOGOFF);
}

int	sm_set_programs(const t_smprogs *p)
{
	static char	names[SM_PROGS_MAX][SM_PROG_NAME_MAX];
	uint32_t	i;

	if (!p || p->count == 0 || p->count > SM_PROGS_MAX)
		return (E_INVAL);
	i = 0;
	while (i < p->count)
	{
		strlcpy(names[i], p->list[i].name, sizeof(names[i]));
		g_items[1 + i].label = names[i];
		i++;
	}
	g_nprogs = p->count;
	return (0);
}

int32_t	sm_prog_of(uint32_t cmd)
{
	if (cmd == SMC_LAUNCH_HELLO)
		return (0);
	if (cmd == SMC_LAUNCH_B)
		return (1);
	if (cmd == SMC_LAUNCH_C)
		return (2);
	return (-1);
}
