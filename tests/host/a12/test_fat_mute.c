#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static t_fakeblk	*g_b;

static void	oracle_negatif(void)
{
	t_fakegeo	g;

	g_b = fakeblk_new(FAKE_SEC_MIN, 512);
	fakefat_format(g_b, 512, 1);
	h_eq_i64("image neuve propre", fake_fsck(g_b), 0);
	fakefat_geo(g_b, &g);
	g_b->img[(uint64_t)(g.rsvd + g.fatsz) * g.bps + 40] = 7;
	h_true(fake_fsck(g_b) > 0, "fsck voit une FAT miroir différente");
	fakefat_format(g_b, 512, 1);
	fakefat_set(g_b, 100, 0x0FFFFFFF);
	h_true(fake_fsck(g_b) > 0, "fsck voit un cluster perdu");
}

static void	mutate_once(uint8_t *img, uint64_t span, uint64_t *seed)
{
	uint32_t	zone;
	uint64_t	at;

	zone = fake_rand(seed) % 4;
	at = fake_rand(seed) % 512;
	if (zone == 1)
		at += 512;
	else if (zone == 2)
		at = 32 * 512 + fake_rand(seed) % 1024;
	else if (zone == 3)
		at = span - 64 * 512 + fake_rand(seed) % (64 * 512);
	img[at] = (uint8_t)fake_rand(seed);
	if (fake_rand(seed) & 1)
		img[at ^ 1] = 0xFF;
}

static void	variante(const uint8_t *snap, uint64_t span, uint64_t *seed)
{
	mutate_once(g_b->img, span, seed);
	if (fakefat_mount(g_b, "/d") == 0)
	{
		fake_walk("/d");
		fake_walk("/d/sous");
		vfs_umount("/d");
	}
	memcpy(g_b->img, snap, span);
}

static void	corpus_mute(void)
{
	uint64_t	seed;
	uint64_t	span;
	uint8_t		*snap;
	uint32_t	i;

	fakefat_format(g_b, 512, 1);
	fakefat_mount(g_b, "/d");
	fake_populate("/d");
	vfs_umount("/d");
	span = (32 + 2 * 521 + 64) * 512;
	snap = malloc(span);
	memcpy(snap, g_b->img, span);
	seed = fake_seed();
	i = 0;
	while (i++ < 400)
		variante(snap, span, &seed);
	h_eq_u64("400 variantes : aucun accès hors disque", g_b->oob, 0);
	free(snap);
	fakeblk_free(g_b);
}

int	main(void)
{
	h_begin("a12/fat_mute");
	h_run("l'oracle fsck détecte les fautes", oracle_negatif);
	h_run("corpus muté à graine", corpus_mute);
	return (h_end());
}
