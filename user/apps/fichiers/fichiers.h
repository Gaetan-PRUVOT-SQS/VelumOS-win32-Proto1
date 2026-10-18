#ifndef FICHIERS_H
# define FICHIERS_H

# include <stdint.h>
# include "../common/uiwin.h"
# include "fichiers_layout.h"
# include "fichiers_model.h"

# define FF_TITLE "Fichiers"
# define FF_ID_FAV_ROOT 20
# define FF_ID_FAV_SYSTEM 21
# define FF_ID_FAV_DATA 22
# define FF_ID_UP 30
# define FF_ID_PATH 31
# define FF_ID_TITLE 32
# define FF_ID_COUNT 33
# define FF_ID_LIST 34
# define FF_ID_NAME 35
# define FF_ID_STATUS 36
# define FF_ID_OPEN 40
# define FF_ID_NEW_DIR 41
# define FF_ID_NEW_FILE 42
# define FF_ID_RENAME 43
# define FF_ID_DELETE 44
# define FF_ID_PROPS 45

typedef struct s_fichiers
{
	t_uiwin		ui;
	t_ffops		ops;
	t_ffdir		dir;
	char		path[VFS_PATH_MAX];
	char		pending[VFS_NAME_MAX + 1];
	int			quit;
}	t_fichiers;

int		ff_app_build(t_fichiers *app);
int		ff_app_goto(t_fichiers *app, const char *path);
void	ff_app_refresh(t_fichiers *app);
void	ff_app_status(t_fichiers *app, const char *text);
void	ff_app_command(void *c, uint32_t code, void *user);
int		ff_app_selection(const t_fichiers *app);
void	ff_app_open_selected(t_fichiers *app);
void	ff_app_action(t_fichiers *app, uint32_t id);
void	ff_app_delete(t_fichiers *app, const char *name);
int		ff_app_open(t_fichiers *app);
void	ff_app_run(t_fichiers *app);

#endif
