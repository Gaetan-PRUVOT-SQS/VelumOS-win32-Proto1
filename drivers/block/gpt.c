#include "blk_int.h"

static bool	gpt_crc_ok(const uint8_t *b, uint32_t hsize)
{
	static const uint8_t	zero[4] = {0, 0, 0, 0};
	uint32_t				st;

	st = crc32_step(CRC32_INIT, b, 16);
	st = crc32_step(st, zero, 4);
	st = crc32_step(st, b + 20, hsize - 20);
	return (~st == rd_le32(b + 16));
}

static void	gpt_fill(const uint8_t *b, const t_gptgeo *g, t_gpthdr *h)
{
	uint64_t	bytes;

	h->my_lba = rd_le64(b + 24);
	h->alt_lba = rd_le64(b + 32);
	h->first_usable = rd_le64(b + 40);
	h->last_usable = rd_le64(b + 48);
	h->entry_lba = rd_le64(b + 72);
	h->nentries = rd_le32(b + 80);
	h->entry_size = rd_le32(b + 84);
	h->array_crc = rd_le32(b + 88);
	bytes = (uint64_t)h->nentries * h->entry_size;
	h->array_bytes = 0;
	h->array_sectors = 0;
	if (bytes <= GPT_ARRAY_MAX)
	{
		h->array_bytes = (uint32_t)bytes;
		h->array_sectors = (uint32_t)((bytes + g->ss - 1) / g->ss);
	}
}

static bool	gpt_layout_ok(const t_gpthdr *h, const t_gptgeo *g)
{
	uint64_t	end;

	if (h->my_lba != g->lba || h->alt_lba >= g->nsectors
		|| h->alt_lba == h->my_lba)
		return (false);
	if (h->first_usable > h->last_usable || h->last_usable >= g->nsectors)
		return (false);
	if (h->my_lba >= h->first_usable && h->my_lba <= h->last_usable)
		return (false);
	if (h->entry_lba == 0 || h->entry_lba >= g->nsectors
		|| h->array_sectors > g->nsectors - h->entry_lba)
		return (false);
	end = h->entry_lba + h->array_sectors - 1;
	if (h->my_lba >= h->entry_lba && h->my_lba <= end)
		return (false);
	return (end < h->first_usable || h->entry_lba > h->last_usable);
}

static bool	gpt_array_ok(const t_gpthdr *h)
{
	if (h->entry_size < GPT_ESZ_MIN || h->entry_size > GPT_ESZ_MAX)
		return (false);
	if (h->entry_size & (h->entry_size - 1))
		return (false);
	if (h->nentries == 0 || h->nentries > GPT_ENTRIES_MAX)
		return (false);
	return (h->array_bytes != 0);
}

int	gpt_header(const uint8_t *b, const t_gptgeo *g, t_gpthdr *h)
{
	uint32_t	hsize;

	memset(h, 0, sizeof(*h));
	if (g->ss < BLK_SS_MIN || g->ss > BLK_SS_MAX || g->nsectors < 3)
		return (E_INVAL);
	if (memcmp(b, "EFI PART", 8) || rd_le32(b + 8) >> 16 != GPT_REVISION_MAJOR)
		return (E_PROTO);
	hsize = rd_le32(b + 12);
	if (hsize < GPT_HDR_MIN || hsize > g->ss || rd_le32(b + 20) != 0)
		return (E_PROTO);
	if (!gpt_crc_ok(b, hsize))
		return (E_PROTO);
	gpt_fill(b, g, h);
	if (!gpt_array_ok(h) || !gpt_layout_ok(h, g))
		return (E_PROTO);
	return (0);
}
