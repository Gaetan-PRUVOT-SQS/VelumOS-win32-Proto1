#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static uint32_t	volume_neuf(uint32_t bad_from, uint32_t next)
{
	t_fakegeo	g;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	fakefat_geo(g_b, &g);
	if (bad_from)
	{
		fakefat_bad_from(g_b, bad_from);
		fake_fsi_set(g_b, FAKE_FSI_FREE, fake_free_real(g_b));
	}
	if (next == FAT_UNKNOWN)
		fake_fsi_set(g_b, FAKE_FSI_FREE, FAT_UNKNOWN);
	else if (next)
		fake_fsi_set(g_b, FAKE_FSI_NEXT, g.nclus + 1);
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	return (g.nclus + 1);
}

static void	dernier_cluster(void)
{
	uint32_t	last;
	uint32_t	next;

	last = volume_neuf(0, 1);
	h_eq_i64("écrire au dernier cluster", vst_put("/d/fin.bin", "x", 1), 0);
	h_true(fake_fat_at(g_b, last) >= FAT_EOC_MIN, "dernier cluster pris");
	h_eq_i64("écart au dernier cluster", fake_free_gap(g_b), 0);
	next = fake_fsi(g_b, FAKE_FSI_NEXT);
	h_true(next == FAT_UNKNOWN || (next >= 2 && next <= last),
		"FSI_Nxt_Free dans le volume ou inconnu");
	h_eq_i64("écrire après le dernier", vst_put("/d/debut.bin", "y", 1), 0);
	h_true(fake_fat_at(g_b, 3) >= FAT_EOC_MIN, "recherche reprise à 2");
	h_eq_i64("écart après reprise", fake_free_gap(g_b), 0);
	h_eq_i64("supprimer le dernier", vfs_unlink("/d/fin.bin"), 0);
	h_eq_u64("dernier cluster rendu", fake_fat_at(g_b, last), 0);
	h_eq_i64("écart après libération du dernier", fake_free_gap(g_b), 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

static void	volume_plein(void)
{
	t_vfile	*f;
	uint8_t	buf[5120];

	volume_neuf(10, 0);
	h_eq_u64("7 clusters libres", fake_free_real(g_b), 7);
	memset(buf, 9, sizeof(buf));
	h_eq_i64("créer", vfs_open("/d/gros.bin", O_CREAT | O_RDWR, &f), 0);
	h_eq_i64("écriture coupée", vfs_pwrite(f, buf, 5120, 0), 7 * 512);
	h_eq_u64("volume plein", fake_free_real(g_b), 0);
	h_eq_i64("écart volume plein", fake_free_gap(g_b), 0);
	h_eq_i64("écrire encore", vfs_pwrite(f, buf, 512, 7 * 512), E_NOSPC);
	h_eq_i64("mkdir sans place", vfs_mkdir("/d/sous", 0), E_NOSPC);
	h_eq_i64("écart après refus", fake_free_gap(g_b), 0);
	h_eq_i64("tronquer à 1", vfs_truncate(f, 1), 0);
	h_eq_u64("6 clusters rendus", fake_free_real(g_b), 6);
	h_eq_i64("écart après troncature", fake_free_gap(g_b), 0);
	h_eq_i64("fermer", vfs_close(f), 0);
	h_eq_i64("supprimer", vfs_unlink("/d/gros.bin"), 0);
	h_eq_u64("7 clusters libres à la fin", fake_free_real(g_b), 7);
	h_eq_i64("écart final", fake_free_gap(g_b), 0);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

static void	compteur_inconnu(void)
{
	volume_neuf(0, FAT_UNKNOWN);
	h_eq_i64("écrire", vst_put("/d/a.bin", "0123456789", 10), 0);
	h_eq_u64("compteur resté inconnu après écriture",
		fake_fsi(g_b, FAKE_FSI_FREE), FAT_UNKNOWN);
	h_eq_i64("supprimer", vfs_unlink("/d/a.bin"), 0);
	h_eq_u64("compteur resté inconnu après suppression",
		fake_fsi(g_b, FAKE_FSI_FREE), FAT_UNKNOWN);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_i64("fsck", fake_fsck(g_b), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12 fat fsinfo limites");
	h_run("FSI_Nxt_Free : dernier cluster du volume", dernier_cluster);
	h_run("FSI_Free_Count : volume plein", volume_plein);
	h_run("FSI_Free_Count : 0xFFFFFFFF inconnu", compteur_inconnu);
	return (h_end());
}
