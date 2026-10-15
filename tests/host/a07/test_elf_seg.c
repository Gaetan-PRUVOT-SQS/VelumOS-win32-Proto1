#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/vmm.h"

static const t_patch	g_seg[] = {
{"filesz > memsz", E_INVAL, {PH0 + P_FILESZ}, {8}, {0x1001}},
{"limite filesz = memsz", 0, {PH2 + P_MEMSZ}, {8}, {0x1000}},
{"memsz nul", E_INVAL, {PH1 + P_MEMSZ, PH1 + P_FILESZ}, {8, 8}, {0, 0}},
{"segment hors fichier de 1", E_INVAL, {PH2 + P_FILESZ}, {8}, {0x1001}},
{"offset non congru", E_INVAL, {PH1 + P_OFFSET}, {8}, {0x1008}},
{"alignement 3", E_INVAL, {PH1 + P_ALIGN}, {8}, {3}},
{"alignement 0x800", E_INVAL, {PH1 + P_ALIGN}, {8}, {0x800}},
{"alignement 0 accepte", 0, {PH1 + P_ALIGN}, {8}, {0}},
{"alignement 2 Mio accepte", 0, {PH1 + P_ALIGN}, {8}, {0x200000}},
{"segment W et X", E_INVAL, {PH2 + P_FLAGS}, {4}, {7}},
{"segment X sans R accepte", 0, {PH0 + P_FLAGS}, {4}, {ELF_PF_X}},
{"page partagee", E_INVAL, {PH1 + P_VADDR, PH1 + P_OFFSET}, {8, 8},
{0x800, 0x800}},
{"segments non croissants", E_INVAL, {PH1 + P_VADDR}, {8}, {0x5000}},
{"fin au-dela de USER_TOP", E_INVAL, {PH2 + P_VADDR}, {8},
{USER_TOP - 0x1000}},
{"debordement 64 bits", E_INVAL, {PH2 + P_VADDR}, {8},
{0xfffffffffffff000ull}},
{"etendue PIE > 64 Mio", E_INVAL, {PH2 + P_VADDR}, {8}, {ELF_SPAN_MAX}},
{"memsz > 64 Mio", E_INVAL, {PH2 + P_MEMSZ}, {8}, {ELF_SPAN_MAX + 1}},
{"PT_INTERP", E_NOTSUP, {PH5 + P_TYPE}, {4}, {ELF_PT_INTERP}},
{"pile executable", E_NOTSUP, {PH5 + P_FLAGS}, {4}, {7}},
{"deux PT_DYNAMIC", E_INVAL, {PH4 + P_TYPE}, {4}, {ELF_PT_DYNAMIC}},
{"aucun PT_LOAD", E_INVAL, {PH0 + P_TYPE, PH1 + P_TYPE, PH2 + P_TYPE},
{4, 4, 4}, {6, 6, 6}},
{"PT_DYNAMIC hors fichier", E_INVAL, {PH3 + P_OFFSET}, {8}, {0x2ff8}}
};

static const t_patch	g_exec[] = {
{"ET_EXEC sous USER_MIN", E_INVAL, {PH0 + P_VADDR, EH_ENTRY}, {8, 8},
{0x8000, 0x8800}},
{"ET_EXEC au-dessus de la zone ASLR", E_INVAL, {PH2 + P_VADDR}, {8},
{ELF_ASLR_HI}},
{"limite ET_EXEC a USER_MIN", 0, {PH0 + P_VADDR, EH_ENTRY}, {8, 8},
{USER_MIN, USER_MIN + 0x800}}
};

static void	seg_table(void)
{
	gen_run(g_seg, sizeof(g_seg) / sizeof(g_seg[0]), ELF_ET_DYN);
	gen_run(g_exec, sizeof(g_exec) / sizeof(g_exec[0]), ELF_ET_EXEC);
}

static void	seg_many(t_gen *g, uint16_t n)
{
	t_elf64_ehdr	eh;
	t_elf64_phdr	ph;
	uint16_t		i;

	gen_valid(g, ELF_ET_DYN);
	gen_get_eh(g, &eh);
	eh.phnum = n;
	gen_set_eh(g, &eh);
	i = 0;
	while (i < n)
	{
		memset(&ph, 0, sizeof(ph));
		ph.type = ELF_PT_LOAD;
		ph.flags = ELF_PF_R | ELF_PF_X;
		ph.vaddr = 0x1000 * (uint64_t)i;
		ph.filesz = 0x100;
		ph.memsz = 0x100;
		gen_set_ph(g, i, &ph);
		i++;
	}
}

static void	seg_count(void)
{
	t_gen		g;
	t_elfinfo	in;

	seg_many(&g, ELF_SEG_MAX);
	gen_patch(&g, EH_ENTRY, 8, 0x80);
	h_eq_i64("16 segments, entree dans le premier", gen_check(&g, &in), 0);
	h_eq_u64("16 segments retenus", in.nseg, ELF_SEG_MAX);
	seg_many(&g, ELF_SEG_MAX + 1);
	gen_patch(&g, EH_ENTRY, 8, 0x80);
	h_eq_i64("17 segments refuses", gen_check(&g, &in), E_INVAL);
}

int	main(void)
{
	h_begin("a07/elf_segments");
	h_run("table des segments", seg_table);
	h_run("nombre de segments", seg_count);
	return (h_end());
}
