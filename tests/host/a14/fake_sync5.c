#include "fake_sync.h"

#define HMARK_MAX 64

static int	g_hmark[HMARK_MAX];
static int	g_hn;

void	fs_h_push(int v)
{
	if (g_hn < HMARK_MAX)
		g_hmark[g_hn] = v;
	g_hn++;
}

int	fs_h_count(void)
{
	return (g_hn);
}

int	fs_h_at(int i)
{
	if (i < 0 || i >= HMARK_MAX)
		return (-1);
	return (g_hmark[i]);
}

void	fs_h_reset(void)
{
	g_hn = 0;
}
