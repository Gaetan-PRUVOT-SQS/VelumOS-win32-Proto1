#include "fake_alloc.h"
#include "stdlib.h"

void	fa_job_drain(t_fa_job *j)
{
	uint32_t	i;

	i = 0;
	while (i < FA_SLOTS)
	{
		if (j->slots[i].p && !fa_intact(j->slots[i].p, j->slots[i].n,
				j->slots[i].seed))
			j->errors++;
		free(j->slots[i].p);
		j->slots[i].p = NULL;
		i++;
	}
}

void	*fa_job_thread(void *arg)
{
	t_fa_job	*j;
	uint32_t	i;

	j = arg;
	i = 0;
	while (i < j->ops)
	{
		fa_job_step(j);
		i++;
	}
	fa_job_drain(j);
	return (NULL);
}
