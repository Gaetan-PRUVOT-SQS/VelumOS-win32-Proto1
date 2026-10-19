#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

#define LABEL_DATA "VELUMDATA  "
#define LABEL_ESP "ESP        "
#define LABEL_AUTRE "NO NAME    "

static t_fakeblk	*nouveau_volume(const char *label)
{
	t_fakeblk	*b;

	b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(b, 512, 1);
	memcpy(b->img + 71, label, 11);
	return (b);
}

static int	data_present(void)
{
	t_vstat	st;

	vfs_boot_init();
	return (vfs_stat("/data", &st) == 0);
}

static void	un_seul_volume(void)
{
	t_fakeblk	*b;

	b = nouveau_volume(LABEL_DATA);
	h_true(data_present(), "étiquette VELUMDATA montée sur /data");
	vfs_umount("/data");
	fakeblk_free(b);
	b = nouveau_volume(LABEL_ESP);
	h_true(!data_present(), "étiquette ESP jamais montée sur /data");
	fakeblk_free(b);
	b = nouveau_volume(LABEL_AUTRE);
	h_true(data_present(), "volume sans étiquette dédiée monté faute de mieux");
	vfs_umount("/data");
	fakeblk_free(b);
}

static void	volume_illisible(void)
{
	t_fakeblk	*b;

	b = nouveau_volume(LABEL_DATA);
	memcpy(b->img + 82, "FAT16   ", 8);
	h_true(!data_present(), "type FAT16 refusé");
	fakeblk_free(b);
	b = nouveau_volume(LABEL_DATA);
	b->img[510] = 0;
	h_true(!data_present(), "signature absente refusée");
	fakeblk_free(b);
	b = nouveau_volume(LABEL_DATA);
	b->dev.nsectors = 0;
	h_true(!data_present(), "disque vide refusé");
	fakeblk_free(b);
}

static void	esp_puis_donnees(void)
{
	t_fakeblk	*esp;
	t_fakeblk	*donnees;

	esp = nouveau_volume(LABEL_ESP);
	donnees = nouveau_volume(LABEL_DATA);
	h_true(data_present(), "ESP en tête, VELUMDATA derrière : /data monté");
	vfs_umount("/data");
	fakeblk_free(donnees);
	fakeblk_free(esp);
}

static void	priorite_etiquette(void)
{
	t_fakeblk	*autre;
	t_fakeblk	*donnees;
	t_vstat		st;

	autre = nouveau_volume(LABEL_AUTRE);
	donnees = nouveau_volume(LABEL_DATA);
	vfs_boot_init();
	h_eq_i64("montage marque", fakefat_mount(donnees, "/m"), 0);
	h_eq_i64("marque écrite", vst_put("/m/MARQUE.TXT", "ok", 2), 0);
	h_eq_i64("démontage marque", vfs_umount("/m"), 0);
	vfs_umount("/data");
	vfs_boot_init();
	h_eq_i64("VELUMDATA préféré au volume sans étiquette",
		vfs_stat("/data/MARQUE.TXT", &st), 0);
	vfs_umount("/data");
	fakeblk_free(donnees);
	fakeblk_free(autre);
}

int	main(void)
{
	h_begin("a12/data_boot");
	h_run("un seul volume", un_seul_volume);
	h_run("volume illisible", volume_illisible);
	h_run("esp puis données", esp_puis_donnees);
	h_run("priorité à l'étiquette", priorite_etiquette);
	return (h_end());
}
