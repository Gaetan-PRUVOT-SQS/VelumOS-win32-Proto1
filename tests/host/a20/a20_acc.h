#ifndef A20_ACC_H
# define A20_ACC_H

# include "velum/libk.h"
# include "accounts.h"

static inline int	acc_load(t_account *a, const char *line)
{
	return (acc_parse_line(line, strlen(line), a));
}

#endif
