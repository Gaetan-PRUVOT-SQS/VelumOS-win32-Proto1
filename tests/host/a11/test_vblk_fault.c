#include <stdio.h>
#include "harness.h"
#include "vblk.h"
#include "fake.h"

static uint8_t	g_buf[4096];

static int	run_op(t_blkdev *d, int op)
{
	if (op == 0)
		return (blk_read(d, 3, 2, g_buf));
	if (op == 1)
		return (blk_write(d, 3, 2, g_buf));
	return (blk_flush(d));
}

static void	fault_case(const t_fkfault *c, uint32_t idx)
{
	t_fkdev		*f;
	t_blkdev	*d;
	char		what[96];

	f = &g_fkdev[0];
	fk_dev_init(f, 256, VBLK_PCI_DEVICE);
	if (vblk_probe(&f->pci, idx) < 0)
		h_true(0, "sonde");
	d = blk_at(blk_count() - 1);
	f->modes = c->modes;
	snprintf(what, sizeof(what), "%s : code", c->what);
	h_eq_i64(what, run_op(d, c->op), c->want);
	f->modes = 0;
	snprintf(what, sizeof(what), "%s : etat ensuite", c->what);
	if (c->dead)
		h_eq_i64(what, blk_read(d, 0, 1, g_buf), E_IO);
	else
		h_eq_i64(what, blk_read(d, 0, 1, g_buf), 0);
	h_eq_i64("verrous rendus", g_fk.locks, 0);
	if (c->dead && !(c->modes & FK_RESET_STUCK))
		h_true(f->status == 0, "appareil remis a zero");
	fk_dev_free(f);
}

static void	fault_table(void)
{
	static const t_fkfault	c[] = {
	{"delai", FK_HANG, 0, E_IO, true}, {"erreur d'etat", FK_IOERR, 0, E_IO,
		false}, {"ecriture en erreur", FK_IOERR, 1, E_IO, false},
	{"indice fou", FK_CRAZY_IDX, 0, E_IO, true},
	{"id faux", FK_BAD_ID, 0, E_IO, true},
	{"longueur courte", FK_SHORT_LEN, 0, E_IO, true},
	{"etat non ecrit", FK_NO_STATUS, 0, E_IO, false},
	{"completion differee", FK_DELAY, 0, 0, false},
	{"vidage sans reponse", FK_HANG, 2, E_IO, true},
	{"remise a zero bloquee", FK_HANG | FK_RESET_STUCK, 0, E_IO, true},
	{NULL, 0, 0, 0, false}};
	int						i;
	int						live;

	i = 0;
	while (c[i].what)
	{
		live = fk_live();
		fault_case(&c[i], (uint32_t)i);
		h_eq_i64("pas d'allocation pendant les E/S", fk_live(), live + 3);
		i++;
	}
	h_true(g_fk.timeouts >= 3, "delais comptes");
}

int	main(void)
{
	h_begin("a11/vblk_fautes");
	h_run("fautes de l'appareil", fault_table);
	return (h_end());
}
