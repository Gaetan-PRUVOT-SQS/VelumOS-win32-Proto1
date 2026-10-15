#include <string.h>
#include "harness.h"
#include "cpu_int.h"

static void	seg_limites(void)
{
	h_eq_u64("kcode SDM", gdt_seg_encode(0, 0xfffff, SEG_KCODE,
			SEG_FLAGS_CODE), 0x00af9a000000ffffull);
	h_eq_u64("kdata SDM", gdt_seg_encode(0, 0xfffff, SEG_KDATA,
			SEG_FLAGS_DATA), 0x00cf92000000ffffull);
	h_eq_u64("udata SDM", gdt_seg_encode(0, 0xfffff, SEG_UDATA,
			SEG_FLAGS_DATA), 0x00cff2000000ffffull);
	h_eq_u64("ucode SDM", gdt_seg_encode(0, 0xfffff, SEG_UCODE,
			SEG_FLAGS_CODE), 0x00affa000000ffffull);
	h_eq_u64("limite 0 base 0", gdt_seg_encode(0, 0, 0, 0), 0);
	h_eq_u64("base max limite max", gdt_seg_encode(0xffffffff, 0xfffff,
			0xff, 0xf), 0xffffffffffffffffull);
	h_eq_u64("limite tronquee a 20 bits", gdt_seg_encode(0, 0xffffffff,
			0, 0), 0x000f00000000ffffull);
	h_eq_u64("base octets 0-2 et 3", gdt_seg_encode(0x12345678, 0, 0, 0),
		0x1200003456780000ull);
}

static void	tss_descripteur(void)
{
	uint64_t	d[2];

	gdt_tss_encode(d, 0xffffffff80123456ull, 103);
	h_eq_u64("tss bas", d[0], 0x8000891234560067ull);
	h_eq_u64("tss haut", d[1], 0x00000000ffffffffull);
	gdt_tss_encode(d, 0, 0);
	h_eq_u64("tss nul bas", d[0], 0x0000890000000000ull);
	h_eq_u64("tss nul haut", d[1], 0);
}

static void	gdt_selecteurs(void)
{
	t_gdt		g;
	uint64_t	tss[2];

	memset(&g, 0xa5, sizeof(g));
	gdt_fill(&g, 0xffffffff80abcdefull);
	gdt_tss_encode(tss, 0xffffffff80abcdefull, sizeof(t_tss) - 1);
	h_eq_u64("entree nulle", g.e[0], 0);
	h_eq_u64("0x08 code noyau", g.e[GDT_KCODE >> 3], 0x00af9a000000ffffull);
	h_eq_u64("0x10 donnees noyau", g.e[GDT_KDATA >> 3],
		0x00cf92000000ffffull);
	h_eq_u64("0x1b donnees user", g.e[GDT_UDATA >> 3], 0x00cff2000000ffffull);
	h_eq_u64("0x23 code user", g.e[GDT_UCODE >> 3], 0x00affa000000ffffull);
	h_eq_u64("0x28 tss bas", g.e[GDT_TSS >> 3], tss[0]);
	h_eq_u64("0x30 tss haut", g.e[(GDT_TSS >> 3) + 1], tss[1]);
	h_eq_u64("rpl 3 user", (GDT_UDATA & 3) + (GDT_UCODE & 3), 6);
	h_eq_u64("sysret ss = base + 8", GDT_UDATA, (GDT_KDATA + 8) | 3);
	h_eq_u64("sysret cs = base + 16", GDT_UCODE, (GDT_KDATA + 16) | 3);
}

static void	idt_porte(void)
{
	static const uint8_t	want[16] = {0x30, 0x5a, 0x08, 0x00, 0x01, 0x8e,
		0x10, 0x80, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0};
	static const uint8_t	want3[16] = {0xff, 0xff, 0x08, 0x00, 0x07, 0xee,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0};
	t_idt_gate				g;

	g = idt_gate_encode(0xffffffff80105a30ull, GDT_KCODE, 1, 0);
	h_true(!memcmp(&g, want, 16), "porte ist1 dpl0 octets SDM");
	g = idt_gate_encode(0xffffffffffffffffull, GDT_KCODE, 0xff, 0xff);
	h_true(!memcmp(&g, want3, 16), "ist et dpl masques, dpl3 = 0xee");
	g = idt_gate_encode(0, 0, 0, 0);
	h_eq_u64("porte nulle attr", g.attr, 0x8e);
	h_eq_u64("porte nulle ist", g.ist, 0);
}

int	main(void)
{
	h_begin("a01/gdt");
	h_run("segments valeurs limites", seg_limites);
	h_run("descripteur tss", tss_descripteur);
	h_run("gdt et selecteurs", gdt_selecteurs);
	h_run("porte idt", idt_porte);
	return (h_end());
}
