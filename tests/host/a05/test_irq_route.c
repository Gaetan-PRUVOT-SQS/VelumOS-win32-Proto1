#include <string.h>
#include "harness.h"
#include "fake.h"
#include "apic_int.h"
#include "irq_int.h"

static t_acpi_info	g_ai;

static void	add_ovr(uint8_t isa, uint32_t gsi, uint16_t flags)
{
	g_ai.override[g_ai.noverrides].isa = isa;
	g_ai.override[g_ai.noverrides].gsi = gsi;
	g_ai.override[g_ai.noverrides].flags = flags;
	g_ai.noverrides++;
}

static void	route_decision(void)
{
	memset(&g_ai, 0, sizeof(g_ai));
	add_ovr(0, 2, 0x00);
	add_ovr(9, 9, 0x0d);
	h_eq_u64("R1 override gagne sur appelant",
		irq_resolve_trig(&g_ai, 9, IRQF_LOW), IRQF_LEVEL);
	h_eq_u64("A=1 override niveau haut", irq_resolve_trig(&g_ai, 9, 0),
		IRQF_LEVEL);
	h_eq_u64("A=0 meme cas sans override", irq_resolve_trig(&g_ai, 8, 0), 0);
	h_eq_u64("R2 B=1 appelant bas", irq_resolve_trig(&g_ai, 5, IRQF_LOW),
		IRQF_LOW);
	h_eq_u64("R2 appelant gsi pci", irq_resolve_trig(&g_ai, 40, IRQF_LEVEL),
		IRQF_LEVEL);
	h_eq_u64("B=0 partage seul", irq_resolve_trig(&g_ai, 5, IRQF_SHARED), 0);
	h_eq_u64("R3 C=1 gsi 15 isa", irq_resolve_trig(&g_ai, 15, 0), 0);
	h_eq_u64("R4 C=0 gsi 16 pci", irq_resolve_trig(&g_ai, 16, 0),
		IRQF_LEVEL | IRQF_LOW);
}

static void	route_mps(void)
{
	memset(&g_ai, 0, sizeof(g_ai));
	add_ovr(0, 2, 0x00);
	add_ovr(10, 10, 0x0f);
	add_ovr(11, 11, 0x07);
	add_ovr(12, 30, 0x03);
	add_ovr(13, 13, 0x02);
	h_eq_u64("mps conforme", irq_resolve_trig(&g_ai, 2, 0), 0);
	h_eq_u64("mps niveau bas", irq_resolve_trig(&g_ai, 10, 0),
		IRQF_LEVEL | IRQF_LOW);
	h_eq_u64("mps front bas", irq_resolve_trig(&g_ai, 11, 0), IRQF_LOW);
	h_eq_u64("mps override gsi haute", irq_resolve_trig(&g_ai, 30, 0),
		IRQF_LOW);
	h_eq_u64("mps polarite reservee", irq_resolve_trig(&g_ai, 13, 0), 0);
}

static void	route_isa(void)
{
	memset(&g_ai, 0, sizeof(g_ai));
	add_ovr(0, 2, 0);
	add_ovr(12, 30, 0);
	h_eq_u64("isa 0 vers gsi 2", irq_isa_lookup(&g_ai, 0), 2);
	h_eq_u64("isa 1 identite", irq_isa_lookup(&g_ai, 1), 1);
	h_eq_u64("isa 12 vers gsi 30", irq_isa_lookup(&g_ai, 12), 30);
	g_ai.noverrides = 1000;
	h_eq_u64("compteur hostile borne", irq_isa_lookup(&g_ai, 15), 15);
}

int	main(void)
{
	h_begin("a05/irq_route");
	h_run("table de decision du declenchement", route_decision);
	h_run("isa vers gsi", route_isa);
	h_run("drapeaux mps", route_mps);
	return (h_end());
}
