#include <stdlib.h>
#include "fake_pth.h"

static void	fh_key_init(void)
{
	if (pthread_key_create(&g_fh.key, NULL) != 0)
		abort();
}

void	*fh_tls_get(void)
{
	pthread_once(&g_fh.once, fh_key_init);
	return (pthread_getspecific(g_fh.key));
}

void	fh_tls_set(void *p)
{
	pthread_once(&g_fh.once, fh_key_init);
	pthread_setspecific(g_fh.key, p);
}
