#include "velum/libk.h"
#include "console.h"

static const t_concmdent	g_table[] = {
{"ls", "liste un dossier : ls [chemin]", con_cmd_ls},
{"cat", "affiche un fichier : cat fichier", con_cmd_cat},
{"cd", "change de dossier : cd [chemin]", con_cmd_cd},
{"pwd", "affiche le dossier courant", con_cmd_pwd},
{"echo", "affiche ses arguments", con_cmd_echo},
{"ps", "liste les processus", con_cmd_ps},
{"kill", "termine un processus lancé d'ici : kill pid", con_cmd_kill},
{"uptime", "durée de fonctionnement, processus, mémoire", con_cmd_uptime},
{"mem", "détail de la mémoire", con_cmd_mem},
{"clear", "efface l'écran", con_cmd_clear},
{"help", "cette liste", con_cmd_help},
{"exit", "ferme la console", con_cmd_exit}
};

const t_concmdent	*con_table(size_t *count)
{
	*count = sizeof(g_table) / sizeof(g_table[0]);
	return (g_table);
}

const t_concmdent	*con_find(const char *name)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_table) / sizeof(g_table[0]))
	{
		if (strcmp(g_table[i].name, name) == 0)
			return (&g_table[i]);
		i++;
	}
	return (NULL);
}
