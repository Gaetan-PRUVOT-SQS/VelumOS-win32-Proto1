#ifndef APKRUN_H
# define APKRUN_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apk.h"
# include "velum/apk/droid.h"
# include "../common/uiwin.h"
# include "apklay.h"

# define APKRUN_W 420
# define APKRUN_H_PX 300
# define APKRUN_ID_HEAD 90
# define APKRUN_ID_MSG 91
# define APKRUN_ID_BASE 100
# define APKRUN_CTL_MAX 256
# define APKRUN_BUDGET 200000000ull
# define APKRUN_MSG_MAX 256
# define APKRUN_LINE_MAX 640
# define APKRUN_TITLE "Application"
# define T_HEAD "Cette application ne peut pas continuer."
# define T_FILE "Le fichier de l'application est illisible."
# define T_APK "Le paquet de l'application est refusé."
# define T_MEM "Mémoire insuffisante pour lancer l'application."
# define T_DEX "Le code de l'application est invalide."
# define T_NOTSUP "L'application emploie une fonction non prise en charge."
# define T_BUDGET "L'application ne répond pas et a été arrêtée."
# define T_VM "La machine de l'application s'est arrêtée sur une erreur."

typedef struct s_apkrun
{
	t_uiwin			ui;
	t_apk			apk;
	t_apkmanifest	man;
	t_apklay		lay;
	t_rect			laid;
	t_droid			*d;
	t_dvm			*vm;
	uint8_t			*file;
	uint8_t			*dex;
	const char		*path;
	char			msg[APKRUN_MSG_MAX];
	uint32_t		click;
	int				known;
	int				started;
	int				dead;
	int				open;
	int				redraw;
	int				quit;
}	t_apkrun;

int			apkrun_load(t_apkrun *app, const char *path);
int			apkrun_open(t_apkrun *app);
void		apkrun_rebuild(t_apkrun *app);
void		apkrun_on_command(void *c, uint32_t code, void *user);
void		apkrun_run(t_apkrun *app);
int			apkrun_check(t_apkrun *app, int r);
int			apkrun_start(t_apkrun *app);
void		apkrun_click(t_apkrun *app, uint32_t view);
void		apkrun_close(t_apkrun *app);
void		apkrun_log_step(t_apkrun *app, const char *what, const char *more);
int			apkrun_refuse(t_apkrun *app, const char *reason);
void		apkrun_applog(void *u, uint32_t lv, const char *tag, const char *m);
uint64_t	apkrun_clock(void *user);

#endif
