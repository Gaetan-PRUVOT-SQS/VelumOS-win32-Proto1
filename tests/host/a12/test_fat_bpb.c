#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static const t_bpbcase	g_cases[] = {
{0, 1, 0x00, E_INVAL, "saut absent"}, {0, 1, 0xE9, 0, "saut E9"},
{510, 1, 0x00, E_INVAL, "signature 55AA"}, {11, 2, 256, E_INVAL, "bps 256"},
{11, 2, 513, E_INVAL, "bps 513"}, {11, 2, 1024, E_INVAL, "bps > disque"},
{13, 1, 0, E_INVAL, "spc 0"}, {13, 1, 3, E_INVAL, "spc non puissance"},
{13, 1, 128, E_NOTSUP, "spc 128 trop peu de clusters"},
{14, 2, 0, E_INVAL, "réservés 0"}, {16, 1, 0, E_INVAL, "aucune FAT"},
{21, 1, 0x12, E_INVAL, "média"}, {17, 2, 512, E_NOTSUP, "RootEntCnt"},
{19, 2, 1000, E_INVAL, "TotSec16"}, {32, 4, 0xFFFFFFFF, E_INVAL, "TotSec32"},
{36, 4, 0, E_INVAL, "FATSz32 0"}, {36, 4, 1, E_INVAL, "FAT trop petite"},
{22, 2, 200, E_INVAL, "FATSz16 non nul"}, {40, 2, 0x83, E_INVAL, "FAT active"},
{40, 2, 0x81, 0, "miroir coupé, FAT 1"}, {42, 2, 0x100, E_NOTSUP, "FSVer"},
{44, 4, 0, E_INVAL, "racine 0"}, {44, 4, 1, E_INVAL, "racine 1"},
{44, 4, 65528, E_INVAL, "racine hors volume"}, {48, 2, 32, E_INVAL, "FSInfo"},
{48, 2, 0xFFFF, 0, "FSInfo absent"}, {0, 0, 0, 0, NULL}
};

static void	bpb_valide(void)
{
	t_fakeblk	*b;
	t_fat		fs;
	t_vstat		st;

	b = fakeblk_new(FAKE_SEC_MIN, 512);
	h_eq_i64("format", fakefat_format(b, 512, 1), 0);
	memset(&fs, 0, sizeof(fs));
	h_eq_i64("analyse", fat_bpb_parse(b->img, FAKE_SEC_MIN * 512, 512, &fs),
		0);
	h_eq_u64("clusters", fs.nclus, 65526);
	h_eq_u64("racine", fs.root, 2);
	h_eq_i64("secteur disque 1024", fat_bpb_parse(b->img, FAKE_SEC_MIN * 512,
			1024, &fs), E_NOTSUP);
	h_eq_i64("montage", fakefat_mount(b, "/d"), 0);
	h_true(vfs_stat("/d", &st) == 0 && (st.mode & S_TYPE_DIR), "racine FAT");
	h_eq_i64("démontage", vfs_umount("/d"), 0);
	h_eq_u64("aucun accès hors disque", b->oob, 0);
	fakeblk_free(b);
}

static void	bs_base(uint8_t *bs)
{
	memset(bs, 0, 512);
	bs[0] = 0xEB;
	bs[2] = 0x90;
	fake_le(bs + 11, 2, 512);
	bs[13] = 1;
	fake_le(bs + 14, 2, 32);
	bs[16] = 2;
	bs[21] = 0xF8;
	fake_le(bs + 32, 4, FAKE_SEC_MIN);
	fake_le(bs + 36, 4, 521);
	fake_le(bs + 44, 4, 2);
	fake_le(bs + 48, 2, 1);
	bs[510] = 0x55;
	bs[511] = 0xAA;
}

static void	table_champs(void)
{
	uint8_t		bs[512];
	t_fat		fs;
	uint32_t	i;

	i = 0;
	while (g_cases[i].what)
	{
		memset(&fs, 0, sizeof(fs));
		bs_base(bs);
		fake_le(bs + g_cases[i].off, g_cases[i].size, g_cases[i].val);
		h_eq_i64(g_cases[i].what, fat_bpb_parse(bs, FAKE_SEC_MIN * 512ull,
				512, &fs), g_cases[i].want);
		i++;
	}
}

int	main(void)
{
	h_begin("a12/fat_bpb");
	h_run("BPB valide et montage", bpb_valide);
	h_run("table de décision des champs", table_champs);
	return (h_end());
}
