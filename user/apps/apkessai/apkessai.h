#ifndef APKESSAI_H
# define APKESSAI_H

# include <stdint.h>
# include "velum/velum.h"

# define ESSAI_SERVER "/system/bin/winsrv"
# define ESSAI_INST "/system/bin/apkinst"
# define ESSAI_RUN "/system/bin/apkrun"
# define ESSAI_SETTLE_NS 500000000ull
# define ESSAI_SHOW_NS 2500000000ull
# define ESSAI_WAIT_NS 20000000000ull

typedef struct s_essai
{
	const char	*prog;
	const char	*arg;
	uint32_t	flags;
	uint32_t	show;
}	t_essai;

int	essai_step(uint32_t rank, const t_essai *e);

#endif
