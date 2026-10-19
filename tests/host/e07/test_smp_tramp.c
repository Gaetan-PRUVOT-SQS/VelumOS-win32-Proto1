#include <string.h>
#include "harness.h"
#include "smp_int.h"

#define PAGE 0x9000ull

static t_tramp_cfg	config(void)
{
	t_tramp_cfg	cfg;

	cfg.base = PAGE;
	cfg.pml4_phys = PAGE + 0x1000;
	cfg.pdpt_phys = PAGE + 0x2000;
	cfg.pd_phys = PAGE + 0x3000;
	cfg.off_pm32 = 0x20;
	cfg.off_lm64 = 0x50;
	cfg.entry = 0xffffffff80123456ull;
	return (cfg);
}

static uint64_t	rd64(const uint8_t *page, uint32_t off)
{
	uint64_t	v;

	memcpy(&v, page + off, sizeof(v));
	return (v);
}

static uint32_t	rd32(const uint8_t *page, uint32_t off)
{
	uint32_t	v;

	memcpy(&v, page + off, sizeof(v));
	return (v);
}

static uint16_t	rd16(const uint8_t *page, uint32_t off)
{
	uint16_t	v;

	memcpy(&v, page + off, sizeof(v));
	return (v);
}

static void	data_layout(void)
{
	static uint8_t	page[4096];
	t_tramp_cfg		cfg;

	cfg = config();
	memset(page, 0xaa, sizeof(page));
	smp_tramp_data(page, &cfg);
	h_eq_u64("gdtr limite = 4 descripteurs", rd16(page, 0x800), 31);
	h_eq_u64("gdtr base", rd32(page, 0x802), PAGE + 0x840);
	h_eq_u64("saut 32 bits, offset", rd32(page, 0x808), PAGE + 0x20);
	h_eq_u64("saut 32 bits, selecteur", rd16(page, 0x80c), 0x18);
	h_eq_u64("saut 64 bits, offset", rd32(page, 0x810), PAGE + 0x50);
	h_eq_u64("saut 64 bits, selecteur", rd16(page, 0x814), 0x08);
	h_eq_u64("cr3", rd64(page, 0x818), PAGE + 0x1000);
	h_eq_u64("point d'entree", rd64(page, 0x828), 0xffffffff80123456ull);
	h_eq_u64("code avant les donnees intact", page[0x7ff], 0xaa);
}

static void	gdt_descriptors(void)
{
	static uint8_t	page[4096];
	t_tramp_cfg		cfg;

	cfg = config();
	smp_tramp_data(page, &cfg);
	h_eq_u64("descripteur nul", rd64(page, 0x840), 0);
	h_eq_u64("code 64 bits SDM 3.4.5", rd64(page, 0x848), 0x00af9a000000ffffull);
	h_eq_u64("donnees SDM 3.4.5", rd64(page, 0x850), 0x00cf92000000ffffull);
	h_eq_u64("code 32 bits SDM 3.4.5", rd64(page, 0x858), 0x00cf9a000000ffffull);
}

static void	target_fields(void)
{
	static uint8_t	page[4096];

	smp_tramp_target(page, 0xffffc00000008ff8ull, 3);
	h_eq_u64("pile", rd64(page, 0x820), 0xffffc00000008ff8ull);
	h_eq_u64("indice", rd64(page, 0x830), 3);
}

static void	tables_layout(void)
{
	static uint64_t	pml4[512];
	static uint64_t	pdpt[512];
	static uint64_t	pd[512];
	static uint64_t	kernel[512];
	t_tramp_cfg		cfg;
	uint32_t		i;

	cfg = config();
	i = 0;
	while (i < 512)
	{
		kernel[i] = 0x1000ull * (i + 1) | 0x3;
		pml4[i] = 0xdead;
		i++;
	}
	smp_tramp_tables(pml4, pdpt, pd, kernel, &cfg);
	h_eq_u64("pml4[0] vers pdpt", pml4[0], (PAGE + 0x2000) | 3);
	h_eq_u64("pdpt[0] vers pd", pdpt[0], (PAGE + 0x3000) | 3);
	h_eq_u64("pd[0] page de 2 Mio, P+RW+PS", pd[0], 0x83);
	h_eq_u64("pml4[1] vide", pml4[1], 0);
	h_eq_u64("pml4[255] vide", pml4[255], 0);
	h_eq_u64("pml4[256] copie", pml4[256], kernel[256]);
	h_eq_u64("pml4[511] copie", pml4[511], kernel[511]);
	h_eq_u64("pdpt[1] vide", pdpt[1], 0);
	h_eq_u64("pd[1] vide", pd[1], 0);
}

int	main(void)
{
	h_begin("e07/smp_tramp");
	h_run("donnees du trampoline", data_layout);
	h_run("gdt du trampoline", gdt_descriptors);
	h_run("cible par coeur", target_fields);
	h_run("tables de pages du trampoline", tables_layout);
	return (h_end());
}
