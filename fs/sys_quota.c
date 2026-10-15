#include "fsys.h"
#include "velum/err.h"
#include "velum/libk.h"

static t_fquota	g_fq;

void	fsys_quota_init(void)
{
	if (g_fq.ready)
		return ;
	memset(&g_fq, 0, sizeof(g_fq));
	spin_init(&g_fq.lock, "vfs.quota");
	g_fq.ready = 1;
}

static t_fqent	*quota_slot(uint32_t pid, int create)
{
	t_fqent		*spare;
	uint32_t	i;

	spare = NULL;
	i = 0;
	while (i < FSYS_QSLOTS)
	{
		if (g_fq.e[i].n && g_fq.e[i].pid == pid)
			return (&g_fq.e[i]);
		if (g_fq.e[i].n == 0 && spare == NULL)
			spare = &g_fq.e[i];
		i++;
	}
	if (create && spare)
		spare->pid = pid;
	if (create)
		return (spare);
	return (NULL);
}

int	fsys_quota_take(uint32_t pid)
{
	t_fqent	*e;
	int		rc;

	fsys_quota_init();
	spin_lock(&g_fq.lock);
	e = quota_slot(pid, 1);
	rc = 0;
	if (e == NULL)
		rc = E_NFILE;
	else if (e->n >= FSYS_FILES_MAX)
		rc = E_MFILE;
	else
		e->n++;
	spin_unlock(&g_fq.lock);
	return (rc);
}

void	fsys_quota_drop(uint32_t pid)
{
	t_fqent	*e;

	fsys_quota_init();
	spin_lock(&g_fq.lock);
	e = quota_slot(pid, 0);
	if (e && e->n)
		e->n--;
	spin_unlock(&g_fq.lock);
}
