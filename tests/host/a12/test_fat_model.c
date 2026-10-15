#include <stdio.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static void	controle_periodique(t_fakeblk *b, t_model *m)
{
	h_eq_i64("relecture complète", model_check(m), 0);
	h_eq_i64("écart FSInfo sans démontage", fake_free_gap(b), 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck périodique", fake_fsck(b), 0);
	h_eq_i64("remontage", fakefat_mount(b, "/d"), 0);
	h_eq_i64("relecture après remontage", model_check(m), 0);
}

static void	vingt_mille(void)
{
	t_fakeblk	*b;
	t_model		m;
	uint32_t	i;

	b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(b, 512, 1);
	h_eq_i64("montage", fakefat_mount(b, "/d"), 0);
	h_eq_i64("sous-dossier", vfs_mkdir("/d/Sous dossier", 0), 0);
	model_init(&m, fake_seed());
	i = 0;
	while (i < 20000)
	{
		model_step(&m);
		if (++i % 2000 == 0)
			controle_periodique(b, &m);
	}
	h_eq_i64("erreurs du modèle", m.errs, 0);
	vfs_umount("/d");
	h_eq_u64("aucun accès hors disque", b->oob, 0);
	model_free(&m);
	fakeblk_free(b);
}

int	main(void)
{
	h_begin("a12/fat_model");
	h_run("20 000 opérations contre un modèle mémoire", vingt_mille);
	return (h_end());
}
