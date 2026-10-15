#include <stddef.h>
#include "a07_fake.h"
#include "harness.h"
#include "velum/err.h"

static const t_patch	g_tags[] = {
{"DT_NEEDED refuse", E_NOTSUP, {DYN0}, {8}, {ELF_DT_NEEDED}},
{"DT_REL refuse", E_NOTSUP, {DYN0}, {8}, {ELF_DT_REL}},
{"DT_RELSZ refuse", E_NOTSUP, {DYN0}, {8}, {ELF_DT_RELSZ}},
{"DT_TEXTREL refuse", E_NOTSUP, {DYN0}, {8}, {ELF_DT_TEXTREL}},
{"DT_JMPREL refuse", E_NOTSUP, {DYN0}, {8}, {ELF_DT_JMPREL}},
{"DT_RELR refuse", E_NOTSUP, {DYN0}, {8}, {ELF_DT_RELR}},
{"DT_RELRSZ refuse", E_NOTSUP, {DYN0}, {8}, {ELF_DT_RELRSZ}},
{"DT_PLTRELSZ nul accepte", 0, {DYN3, DYN3 + 8, PH3 + P_FILESZ}, {8, 8, 8},
{ELF_DT_PLTRELSZ, 0, 0x50}},
{"DT_PLTRELSZ non nul refuse", E_NOTSUP, {DYN3, DYN3 + 8, PH3 + P_FILESZ},
{8, 8, 8}, {ELF_DT_PLTRELSZ, 24, 0x50}},
{"DF_TEXTREL refuse", E_NOTSUP, {DYN3, DYN3 + 8, PH3 + P_FILESZ},
{8, 8, 8}, {ELF_DT_FLAGS, ELF_DF_TEXTREL, 0x50}},
{"DT_FLAGS sans TEXTREL accepte", 0, {DYN3, DYN3 + 8, PH3 + P_FILESZ},
{8, 8, 8}, {ELF_DT_FLAGS, 8, 0x50}},
{"dynamique sans DT_NULL", E_INVAL, {PH3 + P_FILESZ}, {8}, {0x30}},
{"dynamique vide", E_INVAL, {PH3 + P_FILESZ}, {8}, {0}}
};

static const t_patch	g_relas[] = {
{"RELAENT 16", E_INVAL, {DYN2 + 8}, {8}, {16}},
{"RELASZ 47", E_INVAL, {DYN1 + 8}, {8}, {47}},
{"RELASZ 0 sans relocation", 0, {DYN1 + 8}, {8}, {0}},
{"RELASZ sans DT_RELA", E_INVAL, {DYN0}, {8}, {0x6ffffff9}},
{"RELA hors segment", E_INVAL, {DYN0 + 8}, {8}, {0x5000}},
{"RELA dans le bss", E_INVAL, {DYN0 + 8}, {8}, {0x2ff0}},
{"relocation R_X86_64_64", E_NOTSUP, {RELA0 + R_INFO}, {8}, {1}},
{"relocation avec symbole", E_NOTSUP, {RELA0 + R_INFO}, {8},
{(1ull << 32) | ELF_R_RELATIVE}},
{"relocation dans le code", E_INVAL, {RELA1}, {8}, {0x100}},
{"relocation hors segments", E_INVAL, {RELA1}, {8}, {0x9000}},
{"limite cible derniers 8 octets", 0, {RELA1}, {8}, {0x37f8}},
{"limite cible deborde de 1", E_INVAL, {RELA1}, {8}, {0x37f9}},
{"cible 64 bits geante", E_INVAL, {RELA1}, {8}, {0xfffffffffffffffcull}}
};

static void	dyn_tables(void)
{
	gen_run(g_tags, sizeof(g_tags) / sizeof(g_tags[0]), ELF_ET_DYN);
	gen_run(g_relas, sizeof(g_relas) / sizeof(g_relas[0]), ELF_ET_DYN);
}

static void	dyn_relro(void)
{
	t_gen		g;
	t_elfinfo	in;

	gen_valid(&g, ELF_ET_DYN);
	gen_patch(&g, PH4 + P_VADDR, 8, 0x1000);
	h_eq_i64("RELRO hors W accepte", gen_check(&g, &in), 0);
	h_eq_u64("RELRO hors W oublie", in.relro_hi, 0);
	gen_valid(&g, ELF_ET_DYN);
	gen_patch(&g, PH4 + P_MEMSZ, 8, 0xfffffffffffff000ull);
	h_eq_i64("RELRO qui deborde", gen_check(&g, &in), E_INVAL);
}

static void	dyn_base(void)
{
	t_gen		g;
	t_elfinfo	in;
	uint64_t	slots;
	uint64_t	base;

	gen_valid(&g, ELF_ET_DYN);
	h_eq_i64("PIE accepte", gen_check(&g, &in), 0);
	slots = elf_base_slots(&in);
	h_true(slots > 1000000, "creneaux ASLR nombreux");
	h_eq_u64("creneau 0", elf_pick_base(&in, 0), ELF_ASLR_LO);
	h_eq_u64("creneau hors bornes ramene a 0", elf_pick_base(&in, slots),
		ELF_ASLR_LO);
	base = elf_pick_base(&in, slots - 1);
	h_true(base + in.span_hi <= ELF_ASLR_HI, "dernier creneau dans la zone");
	h_eq_u64("base alignee 2 Mio", base % ELF_ASLR_ALIGN, 0);
	h_eq_u64("memoire du PIE", elf_mem_bytes(&in), 0x4000);
}

int	main(void)
{
	h_begin("a07/elf_dynamique");
	h_run("etiquettes et relocations", dyn_tables);
	h_run("RELRO", dyn_relro);
	h_run("base PIE", dyn_base);
	return (h_end());
}
