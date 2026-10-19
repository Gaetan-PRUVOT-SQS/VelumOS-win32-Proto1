#ifndef E15_H
# define E15_H

# include <stddef.h>
# include "console.h"

extern const t_conops	g_fake_ops;
extern uint32_t			g_fake_spawned;
extern char				g_fake_spawn_path[VFS_PATH_MAX];
extern char				g_fake_spawn_arg[64];
extern uint32_t			g_fake_killed;

void	fake_reset(void);
void	screen_text(const t_conscreen *s, char *out, size_t size);
void	run_line(t_console *c, const char *line);

#endif
