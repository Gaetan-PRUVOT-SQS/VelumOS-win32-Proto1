#include "velum/err.h"
#include "velum/libk.h"
#include "../common/tbuf.h"
#include "fichiers.h"

void	ff_app_status(t_fichiers *app, const char *text)
{
	ctl_set_text(&app->ui.root, ctl_find(&app->ui.root, FF_ID_STATUS), text);
	uiwin_flush(&app->ui);
}

static void	show_header(t_fichiers *app)
{
	t_ctlroot	*r;
	const char	*title;
	char		count[FF_SIZE_TEXT_MAX];

	r = &app->ui.root;
	title = ff_dir_title(app->path);
	ff_count_text(count, sizeof(count), app->dir.count);
	ctl_set_text(r, ctl_find(r, FF_ID_PATH), app->path);
	ctl_set_text(r, ctl_find(r, FF_ID_TITLE), title);
	ctl_set_text(r, ctl_find(r, FF_ID_COUNT), count);
}

static void	fill_list(t_fichiers *app)
{
	t_ctl		*list;
	char		label[VFS_NAME_MAX + 2];
	uint32_t	i;

	list = ctl_find(&app->ui.root, FF_ID_LIST);
	ctl_list_clear(&app->ui.root, list);
	i = 0;
	while (i < app->dir.count)
	{
		strlcpy(label, app->dir.items[i].name, sizeof(label));
		if (app->dir.items[i].is_dir)
			strlcat(label, "/", sizeof(label));
		if (ctl_list_add(&app->ui.root, list, label) < 0)
			break ;
		i++;
	}
}

void	ff_app_refresh(t_fichiers *app)
{
	fill_list(app);
	show_header(app);
	app->pending[0] = '\0';
	uiwin_flush(&app->ui);
}

int	ff_app_goto(t_fichiers *app, const char *path)
{
	int	r;

	r = ff_load(&app->ops, path, &app->dir);
	if (r < 0)
	{
		ff_load(&app->ops, app->path, &app->dir);
		ff_app_refresh(app);
		ff_app_status(app, ff_error_text(r));
		return (r);
	}
	strlcpy(app->path, path, sizeof(app->path));
	ctl_set_text(&app->ui.root, ctl_find(&app->ui.root, FF_ID_NAME), "");
	ff_app_refresh(app);
	ff_app_status(app, app->dir.truncated ? "Liste tronquée." : "");
	return (0);
}

int	ff_app_selection(const t_fichiers *app)
{
	t_ctlroot	*r;
	int			i;

	r = (t_ctlroot *)&app->ui.root;
	i = ctl_list_selected(ctl_find(r, FF_ID_LIST));
	if (i < 0 || (uint32_t)i >= app->dir.count)
		return (-1);
	return (i);
}

void	ff_app_open_selected(t_fichiers *app)
{
	char	path[VFS_PATH_MAX];
	char	text[FF_PROPS_MAX];
	int		i;

	i = ff_app_selection(app);
	if (i < 0)
		return (ff_app_status(app, "Aucune sélection."));
	if (ff_path_join(path, sizeof(path), app->path,
			app->dir.items[i].name) < 0)
		return (ff_app_status(app, ff_error_text(E_RANGE)));
	if (app->dir.items[i].is_dir)
	{
		ff_app_goto(app, path);
		return ;
	}
	if (ff_props(&app->ops, app->path, app->dir.items[i].name, text,
			sizeof(text)) < 0)
		return (ff_app_status(app, ff_error_text(E_NOENT)));
	ff_app_status(app, text);
}
