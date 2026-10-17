#ifndef HELLO_H
# define HELLO_H

# include <stdint.h>
# include "../common/uiwin.h"
# include "hello_layout.h"

# define HELLO_ID_TITLE 10
# define HELLO_ID_UPTIME 11
# define HELLO_ID_NOTE 12
# define HELLO_TITLE "Bonjour"

typedef struct s_hello
{
	t_uiwin		ui;
	t_handle	timer;
	int			quit;
}	t_hello;

int		hello_build(t_hello *app);
void	hello_tick(t_hello *app);
void	hello_on_command(void *c, uint32_t code, void *user);
int		hello_open(t_hello *app);
void	hello_run(t_hello *app);

#endif
