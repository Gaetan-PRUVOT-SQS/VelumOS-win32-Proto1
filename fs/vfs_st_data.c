#include "vfs_st.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/libk.h"

static int	data_read(void)
{
	int	fails;

	fails = vst_check(vst_same("/data/LISEZMOI.TXT", "VelumOS FAT32\n", 14)
			== 0, "lecture FAT 8.3");
	fails += vst_check(vst_same("/data/Dossier long/Fichier au nom long.txt",
				"nom long\n", 9) == 0, "lecture FAT LFN");
	return (fails);
}

static void	motif_long(uint8_t *pat)
{
	uint32_t	i;

	i = 0;
	while (i < VST_PATTERN_LEN)
	{
		pat[i] = (uint8_t)((i * 7 + i / 256) % 251);
		i++;
	}
}

static int	data_write(uint8_t *pat)
{
	int	rc;
	int	fails;

	motif_long(pat);
	rc = vfs_mkdir("/data/velum", 0);
	fails = vst_check(rc == 0 || rc == E_EXIST, "mkdir FAT");
	fails += vst_check(vst_put("/data/velum/ecrit par velum.txt", pat,
				VST_PATTERN_LEN) == 0, "écriture FAT");
	fails += vst_check(vst_same("/data/velum/ecrit par velum.txt", pat,
				VST_PATTERN_LEN) == 0, "relecture FAT");
	return (fails);
}

static int	data_rename(void)
{
	t_vstat	st;
	int		fails;

	fails = vst_check(vst_put("/data/velum/tmp.bin", "0123456789", 10) == 0,
			"création FAT");
	fails += vst_check(vfs_rename("/data/velum/tmp.bin",
				"/data/velum/Renommé.bin") == 0, "renommage FAT");
	fails += vst_check(vfs_stat("/data/velum/tmp.bin", &st) == E_NOENT,
			"ancien nom absent");
	fails += vst_check(vfs_unlink("/data/velum/Renommé.bin") == 0,
			"suppression FAT");
	return (fails);
}

#define ETAT_LEN 3000
#define ETAT_DOSSIER "/data/etat"
#define ETAT_BINAIRE "/data/etat/persistance.bin"
#define ETAT_REGLAGES "/data/etat/reglages.conf"
#define ETAT_TEXTE "theme=sombre\ntransparence=reduite\n"

static void	etat_motif(uint8_t *buf)
{
	uint32_t	i;

	i = 0;
	while (i < ETAT_LEN)
	{
		buf[i] = (uint8_t)((i * 13 + 5) % 253);
		i++;
	}
}

static int	etat_ecrire(uint8_t *buf)
{
	int	rc;
	int	fails;

	etat_motif(buf);
	rc = vfs_mkdir(ETAT_DOSSIER, 0);
	fails = vst_check(rc == 0 || rc == E_EXIST, "mkdir état");
	fails += vst_check(vst_put(ETAT_BINAIRE, buf, ETAT_LEN) == 0,
			"écriture état binaire");
	fails += vst_check(vst_put(ETAT_REGLAGES, ETAT_TEXTE,
				strlen(ETAT_TEXTE)) == 0, "écriture réglages");
	if (fails == 0)
		klog_info("vfs: persistance écrite");
	return (fails);
}

static int	etat_relire(uint8_t *buf)
{
	int	fails;

	etat_motif(buf);
	fails = vst_check(vst_same(ETAT_BINAIRE, buf, ETAT_LEN) == 0,
			"relecture état binaire");
	fails += vst_check(vst_same(ETAT_REGLAGES, ETAT_TEXTE,
				strlen(ETAT_TEXTE)) == 0, "relecture réglages");
	motif_long(buf);
	fails += vst_check(vst_same("/data/velum/ecrit par velum.txt", buf,
				VST_PATTERN_LEN) == 0, "fichier long relu");
	if (fails == 0)
		klog_info("vfs: persistance relue ok");
	return (fails);
}

static int	data_persistance(const char *mode, uint8_t *pat)
{
	int	fails;

	if (strcmp(mode, "ecrire") == 0)
	{
		fails = data_write(pat);
		fails += data_rename();
		return (fails + etat_ecrire(pat));
	}
	if (strcmp(mode, "relire") == 0)
	{
		return (etat_relire(pat));
	}
	return (vst_check(0, "mode donnees inconnu"));
}

int	vst_data(void)
{
	t_vstat		st;
	uint8_t		*pat;
	const char	*mode;
	int			fails;

	if (vfs_stat("/data", &st) < 0)
	{
		klog_info("vfs: /data absent, autotest FAT sauté");
		return (0);
	}
	pat = kmalloc_tag(VST_PATTERN_LEN, HEAP_FS);
	if (pat == NULL)
		return (1);
	mode = boot_cmdline_get("donnees");
	if (mode != NULL)
		fails = data_persistance(mode, pat);
	else
	{
		fails = data_read();
		fails += data_write(pat);
		fails += data_rename();
	}
	kfree(pat);
	if (fails == 0 && mode == NULL)
		klog_info("vfs: autotest data ok");
	return (fails);
}
