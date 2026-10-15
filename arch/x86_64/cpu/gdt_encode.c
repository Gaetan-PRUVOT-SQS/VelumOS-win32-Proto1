#include "velum/libk.h"
#include "cpu_int.h"

uint64_t	gdt_seg_encode(uint32_t base, uint32_t limit, uint8_t access,
				uint8_t flags)
{
	uint64_t	d;

	d = limit & 0xffff;
	d |= (uint64_t)(base & 0xffffff) << 16;
	d |= (uint64_t)access << 40;
	d |= (uint64_t)((limit >> 16) & 0xf) << 48;
	d |= (uint64_t)(flags & 0xf) << 52;
	d |= (uint64_t)((base >> 24) & 0xff) << 56;
	return (d);
}

void	gdt_tss_encode(uint64_t out[2], uint64_t base, uint32_t limit)
{
	out[0] = gdt_seg_encode((uint32_t)base, limit, SEG_TSS, 0);
	out[1] = base >> 32;
}

void	gdt_fill(t_gdt *gdt, uint64_t tss_base)
{
	gdt->e[0] = 0;
	gdt->e[GDT_KCODE >> 3] = gdt_seg_encode(0, SEG_LIMIT_MAX, SEG_KCODE,
			SEG_FLAGS_CODE);
	gdt->e[GDT_KDATA >> 3] = gdt_seg_encode(0, SEG_LIMIT_MAX, SEG_KDATA,
			SEG_FLAGS_DATA);
	gdt->e[GDT_UDATA >> 3] = gdt_seg_encode(0, SEG_LIMIT_MAX, SEG_UDATA,
			SEG_FLAGS_DATA);
	gdt->e[GDT_UCODE >> 3] = gdt_seg_encode(0, SEG_LIMIT_MAX, SEG_UCODE,
			SEG_FLAGS_CODE);
	gdt_tss_encode(&gdt->e[GDT_TSS >> 3], tss_base, sizeof(t_tss) - 1);
}

void	tss_fill(t_tss *tss, uint64_t ist1, uint64_t ist2, uint64_t ist3)
{
	memset(tss, 0, sizeof(*tss));
	tss->ist[IST_DOUBLE_FAULT - 1] = ist1;
	tss->ist[IST_NMI - 1] = ist2;
	tss->ist[IST_MCE - 1] = ist3;
	tss->iomap_base = sizeof(t_tss);
}

t_idt_gate	idt_gate_encode(uint64_t fn, uint16_t sel, uint8_t ist, uint8_t dpl)
{
	t_idt_gate	g;

	g.off_lo = (uint16_t)fn;
	g.sel = sel;
	g.ist = ist & 0x7;
	g.attr = GATE_INTR | ((dpl & 0x3) << 5);
	g.off_mid = (uint16_t)(fn >> 16);
	g.off_hi = (uint32_t)(fn >> 32);
	g.zero = 0;
	return (g);
}
