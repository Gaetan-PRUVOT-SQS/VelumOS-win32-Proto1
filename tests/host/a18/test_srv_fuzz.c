#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "help.h"

#define NCLI 4

static t_wsrv	g_s;

static void	revive(t_handle *cl, uint64_t *st)
{
	int		i;
	uint8_t	probe[8];

	i = 0;
	while (i < NCLI)
	{
		hr_drain(cl[i], 0, NULL);
		if (cl[i] == 0 || hr_recv(cl[i], probe, NULL) == E_PIPE)
		{
			if (cl[i] != 0)
				fk_close(cl[i]);
			cl[i] = hr_connect(&g_s);
			if (hg_below(st, 4) != 0)
				hr_hello(&g_s, cl[i]);
		}
		i++;
	}
}

static int	pump(t_handle c, const uint8_t *msg, uint32_t len)
{
	int	r;
	int	k;

	r = fk_send(c, msg, len, 0);
	k = 0;
	while (k++ < 4)
		ws_step(&g_s);
	return (r);
}

static void	fuzz(void)
{
	t_handle	cl[NCLI];
	uint64_t	st;
	uint8_t		*buf;
	int			i;
	int			bad;

	st = hg_seed("a18/srv_fuzz");
	hs_start(&g_s, 320, 240);
	buf = malloc(512);
	memset(cl, 0, sizeof(cl));
	bad = 0;
	i = 0;
	while (i < 100000 && buf != NULL)
	{
		if (i % 64 == 0)
			revive(cl, &st);
		pump(cl[hg_below(&st, NCLI)], buf, hf_message(&g_s, buf, &st));
		bad += (wz_check(&g_s.t) != 0);
		i++;
	}
	h_eq_i64("invariants apres 100 000 messages", bad, 0);
	h_true(g_s.violations > 0, "clients fautifs bien rejetes");
	free(buf);
	hs_stop(&g_s);
	h_eq_i64("rien ne fuit", fk_live(), 0);
}

int	main(void)
{
	h_begin("a18/srv_fuzz");
	h_run("100 000 messages", fuzz);
	return (h_end());
}
