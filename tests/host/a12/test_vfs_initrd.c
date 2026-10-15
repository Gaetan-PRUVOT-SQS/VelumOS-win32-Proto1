#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static uint8_t	g_raw[4096];

static void	montage(void)
{
	t_vbuf	b;

	b.p = g_raw;
	b.len = 0;
	b.cap = sizeof(g_raw);
	fakecpio_valid(&b);
	h_eq_i64("montage", vfs_mount_initrd("/r", g_raw, b.len), 0);
	h_eq_i64("montage double", vfs_mount_initrd("/r", g_raw, b.len), E_BUSY);
}

static void	liste(void)
{
	t_vfile		*f;
	t_dirent	d;
	char		names[256];
	t_vstat		st;

	h_eq_i64("ouvrir dossier", vfs_open("/r", O_RDONLY | O_DIRECTORY, &f), 0);
	names[0] = '\0';
	while (vfs_readdir(f, &d) == 1)
	{
		if (d.name[0] == 'l')
			fake_append(names, sizeof(names), "long");
		else
			fake_append(names, sizeof(names), d.name);
		fake_append(names, sizeof(names), ",");
	}
	h_eq_str("liste triée sans doublon", names, "long,system,vide,x,");
	h_eq_i64("fin stable", vfs_readdir(f, &d), 0);
	h_eq_i64("rembobinage", vfs_seek(f, 0, SEEK_SET_), 0);
	h_true(vfs_readdir(f, &d) == 1 && vfs_stat("/r/l", &st) == E_NOENT,
		"relecture après rembobinage");
	vfs_close(f);
	h_true(vfs_stat("/r/x", &st) == 0 && (st.mode & S_TYPE_DIR)
		&& (st.flags & VFS_STAT_RO), "stat dossier implicite lecture seule");
}

static void	lecture_et_positions(void)
{
	t_vfile	*f;
	char	buf[16];

	h_eq_i64("ouvrir", vfs_open("/r/system/bin/hello", O_RDONLY, &f), 0);
	h_eq_i64("lire 3", vfs_read(f, buf, 3), 3);
	h_eq_i64("lire reste", vfs_read(f, buf + 3, 16), 4);
	h_true(memcmp(buf, "nouveau", 7) == 0, "contenu");
	h_eq_i64("fin de fichier", vfs_read(f, buf, 4), 0);
	h_eq_i64("seek fin -2", vfs_seek(f, -2, SEEK_END_), 5);
	h_eq_i64("seek cur +1", vfs_seek(f, 1, SEEK_CUR_), 6);
	h_eq_i64("seek négatif", vfs_seek(f, -7, SEEK_CUR_), E_INVAL);
	h_eq_i64("seek débordement", vfs_seek(f, INT64_MAX, SEEK_CUR_), E_INVAL);
	h_eq_i64("whence inconnu", vfs_seek(f, 0, 9), E_INVAL);
	h_eq_i64("pread", vfs_pread(f, buf, 2, 2), 2);
	h_eq_i64("pread au-delà", vfs_pread(f, buf, 2, 1000), 0);
	h_eq_i64("pread offset > 2^63", vfs_pread(f, buf, 2, 1ull << 63),
		E_INVAL);
	h_eq_i64("readdir sur fichier", vfs_readdir(f, (t_dirent *)buf),
		E_NOTDIR);
	vfs_close(f);
	h_eq_i64("ouvrir dossier sans O_DIRECTORY", vfs_open("/r/x", 0, &f), 0);
	h_eq_i64("lire un dossier", vfs_read(f, buf, 1), E_ISDIR);
	vfs_close(f);
	h_eq_i64("O_DIRECTORY sur fichier", vfs_open("/r/vide",
			O_DIRECTORY, &f), E_NOTDIR);
}

static void	demontage_et_boot(void)
{
	t_vfile	*f;
	t_vstat	st;
	t_vbuf	b;

	h_eq_i64("ouvrir", vfs_open("/r/vide", O_RDONLY, &f), 0);
	h_eq_i64("démontage occupé", vfs_umount("/r"), E_BUSY);
	vfs_close(f);
	h_eq_i64("démontage", vfs_umount("/r"), 0);
	h_eq_i64("démontage absent", vfs_umount("/r"), E_INVAL);
	b.p = g_raw;
	b.len = 0;
	b.cap = sizeof(g_raw);
	fakecpio_valid(&b);
	g_raw[3] = 'Z';
	fake_boot()->initrd_phys = (uint64_t)g_raw;
	fake_boot()->initrd_size = b.len;
	h_eq_i64("boot avec initrd hostile", vfs_boot_init(), 0);
	h_true(vfs_stat("/", &st) == 0 && (st.mode & S_TYPE_DIR),
		"racine vide toujours présente");
	h_eq_i64("vide", vfs_stat("/system", &st), E_NOENT);
	h_eq_i64("racine indémontable", vfs_umount("/"), E_BUSY);
	h_eq_i64("type inconnu", vfs_mount("/z", "ntfs", NULL, 0), E_NOTSUP);
	h_eq_i64("drapeau inconnu", vfs_mount("/z", "fat32", NULL, 4), E_INVAL);
}

int	main(void)
{
	h_begin("a12/vfs_initrd");
	h_run("montage", montage);
	h_run("liste", liste);
	h_run("lecture et positions", lecture_et_positions);
	h_run("démontage et boot", demontage_et_boot);
	return (h_end());
}
