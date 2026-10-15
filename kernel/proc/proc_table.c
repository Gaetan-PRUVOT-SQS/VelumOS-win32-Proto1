#include "proc_int.h"
#include "velum/err.h"

static t_proctab	g_tab;

t_proctab	*proc_tab(void)
{
	return (&g_tab);
}

static int	pid_in_use(uint32_t pid)
{
	uint32_t	i;

	i = 0;
	while (i < PROC_MAX)
	{
		if (g_tab.slot[i] && g_tab.slot[i]->pid == pid)
			return (1);
		i++;
	}
	return (0);
}

static uint32_t	pid_next(void)
{
	uint32_t	pid;

	pid = g_tab.next_pid;
	while (pid == 0 || pid_in_use(pid))
		pid++;
	g_tab.next_pid = pid + 1;
	return (pid);
}

int	proc_insert(t_process *p)
{
	uint64_t	fl;
	uint32_t	i;

	fl = spin_lock_irqsave(&g_tab.lock);
	if (g_tab.count >= PROC_MAX)
	{
		spin_unlock_irqrestore(&g_tab.lock, fl);
		return (E_AGAIN);
	}
	i = 0;
	while (g_tab.slot[i])
		i++;
	p->pid = pid_next();
	g_tab.slot[i] = p;
	g_tab.count++;
	spin_unlock_irqrestore(&g_tab.lock, fl);
	return (0);
}

void	proc_remove(t_process *p)
{
	uint64_t	fl;
	uint32_t	i;

	fl = spin_lock_irqsave(&g_tab.lock);
	i = 0;
	while (i < PROC_MAX)
	{
		if (g_tab.slot[i] == p)
		{
			g_tab.slot[i] = NULL;
			g_tab.count--;
		}
		i++;
	}
	spin_unlock_irqrestore(&g_tab.lock, fl);
}
