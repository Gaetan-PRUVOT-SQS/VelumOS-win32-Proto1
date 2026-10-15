#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static void	put_lfn(uint8_t *e, uint8_t ord, uint8_t sum, const char *five)
{
	uint32_t	i;

	memset(e, 0xFF, 32);
	e[0] = ord;
	e[11] = 0x0F;
	e[12] = 0;
	e[13] = sum;
	e[26] = 0;
	e[27] = 0;
	i = 0;
	while (i < 5 && five[i])
	{
		e[1 + 2 * i] = (uint8_t)five[i];
		e[2 + 2 * i] = 0;
		i++;
	}
	if (i < 5)
	{
		e[1 + 2 * i] = 0;
		e[2 + 2 * i] = 0;
	}
}

static void	scene(void)
{
	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	put_lfn(fake_rootent(g_b, 0), 0x41, 0x6E, "Ab.tx");
	memcpy(fake_rootent(g_b, 0) + 14, "t\0\0\0", 4);
	memcpy(fake_rootent(g_b, 1), "AB      TXT\x20", 12);
	put_lfn(fake_rootent(g_b, 2), 0x41, 0x00, "faux");
	memcpy(fake_rootent(g_b, 3), "SHORT   TXT\x20", 12);
	put_lfn(fake_rootent(g_b, 4), 0x01, 0xB5, "x");
	put_lfn(fake_rootent(g_b, 5), 0x42, 0xB5, "y");
	memcpy(fake_rootent(g_b, 6), "SHORT2  TXT\x20", 12);
	put_lfn(fake_rootent(g_b, 7), 0x41, 0x07, "orph");
}

static void	lfn_desordonnees(void)
{
	char		got[256];
	t_dirent	d;
	t_vfile		*f;

	scene();
	h_eq_i64("montage", fakefat_mount(g_b, "/d"), 0);
	vfs_open("/d", O_DIRECTORY, &f);
	got[0] = '\0';
	while (vfs_readdir(f, &d) == 1)
	{
		fake_append(got, sizeof(got), d.name);
		fake_append(got, sizeof(got), ",");
	}
	vfs_close(f);
	h_eq_str("LFN valide, somme fausse, désordre, orphelin", got,
		"Ab.txt,SHORT.TXT,SHORT2.TXT,");
	h_eq_u64("somme de contrôle spec", fat_lfn_sum((const uint8_t *)
			"THEQUI~1FOX"), 0x07);
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/fat_lfn");
	h_run("LFN désordonnées et sommes fausses", lfn_desordonnees);
	return (h_end());
}
