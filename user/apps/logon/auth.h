#ifndef AUTH_H
# define AUTH_H

# include <stddef.h>
# include <stdint.h>
# include "accounts.h"
# include "limiter.h"

typedef enum e_authres
{
	AUTH_OK = 0,
	AUTH_BAD,
	AUTH_WAIT,
	AUTH_DENIED
}	t_authres;

typedef struct s_authreq
{
	const t_accounts	*set;
	t_limiters			*lim;
	uint32_t			index;
	uint64_t			now_ns;
}	t_authreq;

int	auth_attempt(const t_authreq *rq, const char *pw, size_t pwlen,
		uint32_t *wait_s);

#endif
