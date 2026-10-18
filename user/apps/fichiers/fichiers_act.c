#include "velum/err.h"
#include "velum/libk.h"
#include "fichiers.h"

static const char	*edit_text(t_fichiers *app)
{
	return (ctl_find(&app->ui.root, FF_ID_NAME)->text);
}

static void	report(t_fichiers *app, int r, const char *done)
{
	if (r < 0)
		ff_app_status(app, ff_error_text(r));
	else
	{
		ff_app_goto(app, app->path);
		ff_app_status(app, done);
	}
}

static void	go_up(t_fichiers *app)
{
	char	parent[VFS_PATH_MAX];

	strlcpy(parent, app->path, sizeof(parent));
	if (ff_path_parent(parent) < 0)
		return (ff_app_status(app, "Déjà à la racine."));
	ff_app_goto(app, parent);
}

static void	rename_selected(t_fichiers *app)
{
	int	i;
	int	r;

	i = ff_app_selection(app);
	if (i < 0)
		return (ff_app_status(app, "Aucune sélection."));
	r = ff_rename(&app->ops, app->path, app->dir.items[i].name,
			edit_text(app));
	report(app, r, "Renommé.");
}

void	ff_app_delete(t_fichiers *app, const char *name)
{
	if (strcmp(app->pending, name))
	{
		strlcpy(app->pending, name, sizeof(app->pending));
		ff_app_status(app, "Cliquez encore sur Supprimer pour confirmer.");
		return ;
	}
	report(app, ff_remove(&app->ops, app->path, name), "Supprimé.");
}

static void	delete_selected(t_fichiers *app)
{
	int	i;

	i = ff_app_selection(app);
	if (i < 0)
		return (ff_app_status(app, "Aucune sélection."));
	ff_app_delete(app, app->dir.items[i].name);
}

static void	props_selected(t_fichiers *app)
{
	char	text[FF_PROPS_MAX];
	int		i;
	int		r;

	i = ff_app_selection(app);
	if (i < 0)
		return (ff_app_status(app, "Aucune sélection."));
	r = ff_props(&app->ops, app->path, app->dir.items[i].name, text,
			sizeof(text));
	ff_app_status(app, r < 0 ? ff_error_text(r) : text);
}

void	ff_app_action(t_fichiers *app, uint32_t id)
{
	if (id >= FF_ID_FAV_ROOT && id <= FF_ID_FAV_DATA)
		ff_app_goto(app, id == FF_ID_FAV_ROOT ? "/"
			: id == FF_ID_FAV_SYSTEM ? "/system" : "/data");
	else if (id == FF_ID_UP)
		go_up(app);
	else if (id == FF_ID_OPEN)
		ff_app_open_selected(app);
	else if (id == FF_ID_NEW_DIR)
		report(app, ff_make_dir(&app->ops, app->path, edit_text(app)),
			"Dossier créé.");
	else if (id == FF_ID_NEW_FILE)
		report(app, ff_make_file(&app->ops, app->path, edit_text(app)),
			"Fichier créé.");
	else if (id == FF_ID_RENAME)
		rename_selected(app);
	else if (id == FF_ID_DELETE)
		delete_selected(app);
	else if (id == FF_ID_PROPS)
		props_selected(app);
}

void	ff_app_command(void *c, uint32_t code, void *user)
{
	const t_ctl	*ctl;
	t_fichiers	*app;
	int			i;

	ctl = c;
	app = user;
	if (!app)
		return ;
	if (code == CN_CLICKED)
		ff_app_action(app, ctl->id);
	else if (ctl->id == FF_ID_LIST && code == CN_DBLCLK)
		ff_app_open_selected(app);
	else if (ctl->id == FF_ID_LIST && code == CN_SELECT)
	{
		i = ff_app_selection(app);
		if (i >= 0)
			ctl_set_text(&app->ui.root, ctl_find(&app->ui.root,
					FF_ID_NAME), app->dir.items[i].name);
	}
}
