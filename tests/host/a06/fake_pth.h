#ifndef FAKE_PTH_H
# define FAKE_PTH_H

# include <pthread.h>
# include "fake_host.h"

typedef struct s_fhglob
{
	pthread_mutex_t	mtx;
	pthread_cond_t	cond;
	pthread_once_t	once;
	pthread_key_t	key;
}	t_fhglob;

typedef struct s_fhstart
{
	pthread_t	th;
	t_fhfn		fn;
	void		*arg;
}	t_fhstart;

extern t_fhglob	g_fh;

#endif
