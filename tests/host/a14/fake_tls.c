#include "fake_int.h"
#include "fake_sys.h"

static t_fslot			g_fslots[FAKE_SLOTS];
static uint32_t			g_fslot_next;
static pthread_key_t	g_fkey;
static pthread_once_t	g_fonce = PTHREAD_ONCE_INIT;

static void	fake_key_init(void)
{
	pthread_key_create(&g_fkey, NULL);
}

t_fslot	*fake_slot(void)
{
	t_fslot		*s;
	uint32_t	i;

	pthread_once(&g_fonce, fake_key_init);
	s = pthread_getspecific(g_fkey);
	if (s)
		return (s);
	i = __atomic_fetch_add(&g_fslot_next, 1, __ATOMIC_SEQ_CST);
	s = &g_fslots[i % FAKE_SLOTS];
	s->def.self = &s->def;
	s->override = NULL;
	s->tid = 0;
	pthread_setspecific(g_fkey, s);
	return (s);
}

t_vtcb	*v_tcb(void)
{
	t_fslot	*s;

	s = fake_slot();
	if (s->override)
		return (s->override);
	return (&s->def);
}

int	*__velum_errno(void)
{
	return (&v_tcb()->err);
}

void	fake_set_fs(void *tcb)
{
	fake_slot()->override = tcb;
}
