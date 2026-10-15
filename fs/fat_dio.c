#include "fat.h"
#include "vfs_weak.h"
#include "velum/err.h"

uint64_t	fat_clus_sec(const t_fat *fs, uint32_t c)
{
	return (fs->data_start + (uint64_t)(c - 2) * fs->spc);
}

uint64_t	fat_clus_byte(const t_fat *fs, uint32_t c)
{
	return (fat_clus_sec(fs, c) * fs->bps);
}

int	fat_dread(t_fat *fs, uint64_t sec, uint32_t n, uint8_t *dst)
{
	if (sec >= fs->tot_sec || n > fs->tot_sec - sec)
		return (E_IO);
	if (blk_read(fs->dev, sec * fs->ratio, n * fs->ratio, dst) < 0)
		return (E_IO);
	return (0);
}

int	fat_dwrite(t_fat *fs, uint64_t sec, uint32_t n, const uint8_t *s)
{
	if (fs->ro)
		return (E_PERM);
	if (sec >= fs->tot_sec || n > fs->tot_sec - sec)
		return (E_IO);
	fat_cache_drop(fs, sec, n);
	if (blk_write(fs->dev, sec * fs->ratio, n * fs->ratio, s) < 0)
		return (E_IO);
	return (0);
}

int	fat_zero_clus(t_fat *fs, uint32_t c)
{
	if (!fat_ok(fs, c))
		return (E_IO);
	return (fat_dwrite(fs, fat_clus_sec(fs, c), fs->spc, fs->zero));
}
