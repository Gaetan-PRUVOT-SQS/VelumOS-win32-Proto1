#include "velum/abi/abi_input.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "shell.h"
#include "sh_launch_apk.h"

static t_launchapks	g_apks;

static void	apks_load(void)
{
	static char	buf[SH_LAUNCH_REG_BUF];
	size_t		len;

	g_apks.count = 0;
	len = 0;
	if (os_read_file(SH_LAUNCH_REG_SYSTEM, buf, sizeof(buf), &len) == 0)
		sh_launch_apk_parse(buf, len, SH_LAUNCH_ORIGIN_SYSTEM, &g_apks);
	len = 0;
	if (os_read_file(SH_LAUNCH_REG_DATA, buf, sizeof(buf), &len) == 0)
		sh_launch_apk_parse(buf, len, SH_LAUNCH_ORIGIN_DATA, &g_apks);
}

static void	app_set(t_lmapp *a, const char *label, uint32_t id, t_iconid icon)
{
	memset(a, 0, sizeof(*a));
	strlcpy(a->label, label, sizeof(a->label));
	a->id = id;
	a->icon = icon;
}

void	sh_launch_ui_refresh(t_shell *sh)
{
	static t_lmapp	apps[LUNA_LM_APPS_MAX];
	uint32_t		n;
	uint32_t		i;

	apks_load();
	n = 0;
	i = 0;
	while (i < sh->progs.count && n < LUNA_LM_APPS_MAX)
	{
		app_set(&apps[n++], sh->progs.list[i].name, i, ICON_PROGRAM);
		i++;
	}
	if (n < LUNA_LM_APPS_MAX)
		app_set(&apps[n++], "Ex\xc3\xa9" "cuter...", SH_LID_RUN, ICON_RUN);
	i = 0;
	while (i < g_apks.count && n < LUNA_LM_APPS_MAX)
	{
		app_set(&apps[n++], g_apks.list[i].label, SH_LID_APK_BASE + i,
			ICON_PROGRAM);
		i++;
	}
	luna_lm_set_apps(&sh->lmodel, apps, n);
}

void	sh_launch_ui_open(t_shell *sh)
{
	luna_lm_clear_query(&sh->lmodel);
	luna_lm_set_tab(&sh->lmodel, LUNA_LV_PINNED);
	sh->lmodel.hot = LUNA_LM_NONE;
	sh_launch_ui_refresh(sh);
}

void	sh_launch_ui_note(t_shell *sh, uint32_t id)
{
	luna_lm_touch(&sh->lmodel, id);
}

static void	run_apk(t_shell *sh, uint32_t id)
{
	char	path[SH_LAUNCH_PKG_MAX + 32];

	if (id - SH_LID_APK_BASE >= g_apks.count)
		return ;
	if (sh_launch_apk_path(&g_apks.list[id - SH_LID_APK_BASE], path,
			sizeof(path)) < 0)
		return ;
	sh_launch_arg(sh, SH_APKRUN_PATH, path);
}

static void	run_id(t_shell *sh, uint32_t id)
{
	luna_lm_touch(&sh->lmodel, id);
	sh_menu_close(sh);
	if (id == SH_LID_RUN)
		sh_dlg_open(sh, SH_DLG_RUN, "Ex\xc3\xa9" "cuter");
	else if (id >= SH_LID_APK_BASE)
		run_apk(sh, id);
	else if (id < sh->progs.count)
		sh_launch_arg(sh, sh->progs.list[id].path, sh->progs.list[id].arg);
}

int	sh_launch_ui_char(t_shell *sh, uint32_t codepoint)
{
	if (!sh->sm.open)
		return (0);
	if (!luna_lm_type(&sh->lmodel, codepoint))
		return (1);
	sh->sm.focus = SM_NO_ITEM;
	sh_menu_paint(sh);
	return (1);
}

static int	key_list(t_shell *sh, uint32_t vk)
{
	t_lmodel	*m;
	uint32_t	id;

	m = &sh->lmodel;
	if (vk == VK_DOWN || vk == VK_UP)
		luna_lm_move(m, vk == VK_DOWN ? 1 : -1);
	else if (vk == VK_NEXT || vk == VK_PRIOR)
		luna_lm_move(m, vk == VK_NEXT ? LUNA_LM_ROWS : -LUNA_LM_ROWS);
	else if (vk == VK_HOME || vk == VK_END)
		luna_lm_move(m, vk == VK_HOME ? -(int32_t)LUNA_LM_APPS_MAX
			: (int32_t)LUNA_LM_APPS_MAX);
	else if (vk == VK_RETURN && luna_lm_selected(m, &id))
	{
		run_id(sh, id);
		return (1);
	}
	else if (vk != VK_RETURN)
		return (0);
	sh_menu_paint(sh);
	return (1);
}

static int	key_back(t_shell *sh, uint32_t vk)
{
	t_lmodel	*m;

	m = &sh->lmodel;
	if (vk == VK_BACK)
		luna_lm_backspace(m);
	else if (vk == VK_ESCAPE && m->qlen > 0)
		luna_lm_clear_query(m);
	else if (vk == VK_ESCAPE && m->tab != LUNA_LV_PINNED)
		luna_lm_set_tab(m, LUNA_LV_PINNED);
	else if ((vk == VK_LEFT || vk == VK_RIGHT) && m->qlen == 0)
		luna_lm_set_tab(m, m->tab + (vk == VK_RIGHT ? 1 : -1));
	else
		return (0);
	sh_menu_paint(sh);
	return (1);
}

int	sh_launch_ui_key(t_shell *sh, uint32_t vk)
{
	if (!sh->sm.open)
		return (0);
	if (luna_lm_view(&sh->lmodel) == LUNA_LV_PINNED)
		return (vk == VK_BACK && key_back(sh, vk));
	if (key_back(sh, vk))
		return (1);
	return (key_list(sh, vk));
}

static int	mouse_move(t_shell *sh, const t_wmmouse *m)
{
	int32_t	row;

	row = luna_launcher_row_hit(&sh->sml, m->x, m->y);
	if (luna_lm_row(&sh->lmodel, row) == NULL)
		row = LUNA_LM_NONE;
	if (row != sh->lmodel.hot)
	{
		sh->lmodel.hot = row;
		sh_menu_paint(sh);
	}
	return (luna_lm_view(&sh->lmodel) != LUNA_LV_PINNED);
}

static int32_t	row_x(const t_shell *sh, int32_t x)
{
	t_rect	r;

	r = luna_launcher_row_rect(&sh->sml, 0);
	if (x >= r.x + r.w && x < sh->sml.search.x + sh->sml.search.w)
		return (r.x + r.w - 1);
	return (x);
}

static int	mouse_down(t_shell *sh, const t_wmmouse *m)
{
	int32_t		tab;
	int32_t		row;
	uint32_t	id;

	tab = luna_launcher_tab_hit(&sh->sml, m->x, m->y);
	if (tab >= 0 && sh->lmodel.qlen == 0)
	{
		luna_lm_set_tab(&sh->lmodel, tab);
		sh_menu_paint(sh);
		return (1);
	}
	if (luna_lm_view(&sh->lmodel) == LUNA_LV_PINNED)
		return (0);
	row = luna_launcher_row_hit(&sh->sml, row_x(sh, m->x), m->y);
	if (row < 0)
		return (0);
	if (luna_lm_select(&sh->lmodel, row) >= 0
		&& luna_lm_selected(&sh->lmodel, &id))
		run_id(sh, id);
	return (1);
}

int	sh_launch_ui_mouse(t_shell *sh, const t_wmmouse *m)
{
	if (!sh->sm.open)
		return (0);
	if (m->type == INP_MOUSE_MOVE)
		return (mouse_move(sh, m));
	if (m->type == INP_WHEEL && m->wheel != 0)
	{
		luna_lm_scroll(&sh->lmodel, m->wheel > 0 ? -1 : 1);
		sh_menu_paint(sh);
		return (1);
	}
	if (m->type != INP_MOUSE_DOWN
		|| ((m->buttons >> WM_BTN_SHIFT) & WM_BTN_MASK) != BTN_LEFT)
		return (0);
	return (mouse_down(sh, m));
}
