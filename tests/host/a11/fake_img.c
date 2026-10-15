#include <string.h>
#include "blk_int.h"
#include "fake.h"

#define FK_GPT_ENTRIES 128
#define FK_GPT_ESZ 128

static void	hdr_store(uint8_t *b, const t_gpthdr *h)
{
	memset(b, 0, GPT_HDR_MIN);
	memcpy(b, "EFI PART", 8);
	fk_le32(b + 8, 0x00010000);
	fk_le32(b + 12, GPT_HDR_MIN);
	fk_le64(b + 24, h->my_lba);
	fk_le64(b + 32, h->alt_lba);
	fk_le64(b + 40, h->first_usable);
	fk_le64(b + 48, h->last_usable);
	memset(b + 56, 0x5c, 16);
	fk_le64(b + 72, h->entry_lba);
	fk_le32(b + 80, h->nentries);
	fk_le32(b + 84, h->entry_size);
}

void	fk_gpt_rehash(t_fkram *r, uint64_t lba)
{
	uint8_t		*b;
	uint64_t	elba;
	uint64_t	bytes;
	uint64_t	total;

	total = r->dev.nsectors * r->dev.sector_size;
	b = r->data + lba * r->dev.sector_size;
	elba = rd_le64(b + 72);
	bytes = (uint64_t)rd_le32(b + 80) * rd_le32(b + 84);
	if (elba < r->dev.nsectors && bytes <= total - elba * r->dev.sector_size)
		fk_le32(b + 88, crc32_calc(r->data + elba * r->dev.sector_size,
				bytes));
	fk_le32(b + 16, 0);
	if (rd_le32(b + 12) <= r->dev.sector_size)
		fk_le32(b + 16, crc32_calc(b, rd_le32(b + 12)));
}

static void	entries_store(t_fkram *r, uint64_t elba, const t_fkpart *p,
	uint32_t n)
{
	uint8_t		*e;
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		e = r->data + elba * r->dev.sector_size + p[i].slot * FK_GPT_ESZ;
		memset(e, 0x11, 16);
		memset(e + 16, 0x22 + (int)i, 16);
		fk_le64(e + 32, p[i].first);
		fk_le64(e + 40, p[i].last);
		i++;
	}
}

static void	hdr_both(t_fkram *r, t_gpthdr *h, uint32_t asec)
{
	uint64_t	ns;

	ns = r->dev.nsectors;
	h->my_lba = 1;
	h->alt_lba = ns - 1;
	h->entry_lba = 2;
	hdr_store(r->data + r->dev.sector_size, h);
	fk_gpt_rehash(r, 1);
	h->my_lba = ns - 1;
	h->alt_lba = 1;
	h->entry_lba = ns - 1 - asec;
	hdr_store(r->data + (ns - 1) * r->dev.sector_size, h);
	fk_gpt_rehash(r, ns - 1);
}

void	fk_gpt_build(t_fkram *r, const t_fkpart *p, uint32_t n)
{
	t_gpthdr	h;
	t_fkpart	prot;
	uint64_t	ns;
	uint32_t	asec;

	ns = r->dev.nsectors;
	asec = FK_GPT_ENTRIES * FK_GPT_ESZ / r->dev.sector_size;
	prot.first = 1;
	prot.last = ns - 1;
	fk_mbr_entry(r->data, 0, MBR_TYPE_GPT, &prot);
	memset(&h, 0, sizeof(h));
	h.nentries = FK_GPT_ENTRIES;
	h.entry_size = FK_GPT_ESZ;
	h.first_usable = 2 + asec;
	h.last_usable = ns - 2 - asec;
	entries_store(r, 2, p, n);
	entries_store(r, ns - 1 - asec, p, n);
	hdr_both(r, &h, asec);
}
