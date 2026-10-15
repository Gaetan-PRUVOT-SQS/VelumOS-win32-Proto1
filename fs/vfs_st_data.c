#include "vfs_st.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"

static int	data_read(void)
{
	int	fails;

	fails = vst_check(vst_same("/data/LISEZMOI.TXT", "VelumOS FAT32\n", 14)
			== 0, "lecture FAT 8.3");
	fails += vst_check(vst_same("/data/Dossier long/Fichier au nom long.txt",
				"nom long\n", 9) == 0, "lecture FAT LFN");
	return (fails);
}

static int	data_write(uint8_t *pat)
{
	uint32_t	i;
	int			rc;
	int			fails;

	i = 0;
	while (i < VST_PATTERN_LEN)
	{
		pat[i] = (uint8_t)((i * 7 + i / 256) % 251);
		i++;
	}
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

int	vst_data(void)
{
	t_vstat	st;
	uint8_t	*pat;
	int		fails;

	if (vfs_stat("/data", &st) < 0)
	{
		klog_info("vfs: /data absent, autotest FAT sauté");
		return (0);
	}
	pat = kmalloc_tag(VST_PATTERN_LEN, HEAP_FS);
	if (pat == NULL)
		return (1);
	fails = data_read();
	fails += data_write(pat);
	fails += data_rename();
	kfree(pat);
	if (fails == 0)
		klog_info("vfs: autotest data ok");
	return (fails);
}
