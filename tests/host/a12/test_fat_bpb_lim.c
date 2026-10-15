#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static int	parse_count(uint32_t nclus, uint32_t fatsz)
{
	uint8_t	bs[512];
	t_fat	fs;

	memset(&fs, 0, sizeof(fs));
	memset(bs, 0, 512);
	bs[0] = 0xE9;
	fake_le(bs + 11, 2, 512);
	bs[13] = 1;
	fake_le(bs + 14, 2, 32);
	bs[16] = 2;
	bs[21] = 0xF0;
	fake_le(bs + 32, 4, 32 + 2 * fatsz + nclus);
	fake_le(bs + 36, 4, fatsz);
	fake_le(bs + 44, 4, 2);
	bs[510] = 0x55;
	bs[511] = 0xAA;
	return (fat_bpb_parse(bs, UINT64_MAX, 512, &fs));
}

static void	limites_type(void)
{
	h_eq_i64("4084 clusters (FAT12)", parse_count(4084, 520), E_NOTSUP);
	h_eq_i64("4085 clusters (FAT16)", parse_count(4085, 520), E_NOTSUP);
	h_eq_i64("65524 clusters (FAT16)", parse_count(65524, 520), E_NOTSUP);
	h_eq_i64("65525 clusters (FAT32)", parse_count(65525, 520), 0);
	h_eq_i64("max FAT32", parse_count(0x0FFFFFF5, 2097152), 0);
	h_eq_i64("max + 1", parse_count(0x0FFFFFF6, 2097152), E_INVAL);
}

int	main(void)
{
	h_begin("a12/fat_bpb_lim");
	h_run("limites du type par nombre de clusters", limites_type);
	return (h_end());
}
