#include <stddef.h>
#include "a07_fake.h"
#include "harness.h"
#include "velum/err.h"

static const t_patch	g_hdr[] = {
{"entete magie 0", E_INVAL, {0}, {1}, {0}},
{"entete classe 32 bits", E_INVAL, {4}, {1}, {1}},
{"entete gros boutiste", E_INVAL, {5}, {1}, {2}},
{"entete version ident 0", E_INVAL, {6}, {1}, {0}},
{"entete version 2", E_INVAL, {EH_VERSION}, {4}, {2}},
{"entete type REL", E_INVAL, {EH_TYPE}, {2}, {1}},
{"entete type CORE", E_INVAL, {EH_TYPE}, {2}, {4}},
{"entete machine i386", E_INVAL, {EH_MACHINE}, {2}, {3}},
{"entete ehsize 63", E_INVAL, {EH_EHSIZE}, {2}, {63}},
{"entete phentsize 55", E_INVAL, {EH_PHENTSIZE}, {2}, {55}},
{"entete phnum 0", E_INVAL, {EH_PHNUM}, {2}, {0}},
{"entete phnum 65", E_INVAL, {EH_PHNUM}, {2}, {65}},
{"limite phnum 64 accepte", 0, {EH_PHNUM}, {2}, {64}},
{"entete phoff non aligne", E_INVAL, {EH_PHOFF}, {8}, {65}},
{"entete phoff dans l'en-tete", E_INVAL, {EH_PHOFF}, {8}, {56}},
{"entete phoff hors fichier", E_INVAL, {EH_PHOFF}, {8}, {GEN_SIZE}},
{"table qui deborde de 8 octets", E_INVAL, {EH_PHOFF}, {8}, {GEN_SIZE - 328}},
{"entete phoff geant", E_INVAL, {EH_PHOFF}, {8}, {0xfffffffffffffff8ull}},
{"entree dans un segment R", E_INVAL, {EH_ENTRY}, {8}, {0x1000}},
{"entree hors image", E_INVAL, {EH_ENTRY}, {8}, {0x10000}},
{"limite entree dernier octet X", 0, {EH_ENTRY}, {8}, {0xfff}}
};

static void	hdr_valid_dyn(void)
{
	t_gen		g;
	t_elfinfo	in;

	gen_valid(&g, ELF_ET_DYN);
	h_eq_i64("ET_DYN valide accepte", gen_check(&g, &in), 0);
	h_eq_u64("trois segments", in.nseg, 3);
	h_eq_u64("entree", in.entry, GEN_ENTRY);
	h_eq_u64("phdr en memoire", in.phdr_vaddr, 64);
	h_eq_u64("phnum", in.phnum, GEN_PHNUM);
	h_eq_u64("deux relocations", in.rela_count, 2);
	h_eq_u64("table de relocations", in.rela_off, GEN_RELA_OFF);
	h_eq_u64("etendue basse", in.span_lo, 0);
	h_eq_u64("etendue haute", in.span_hi, 0x4000);
	h_eq_u64("relro garde", in.relro_hi - in.relro_lo, 0x1000);
	h_eq_u64("droits text", in.seg[0].flags, ELF_PF_R | ELF_PF_X);
	h_eq_u64("bss", in.seg[2].memsz - in.seg[2].filesz, 0x800);
}

static void	hdr_valid_exec(void)
{
	t_gen		g;
	t_elfinfo	in;

	gen_valid(&g, ELF_ET_EXEC);
	h_eq_i64("ET_EXEC valide accepte", gen_check(&g, &in), 0);
	h_eq_u64("ET_EXEC etendue", in.span_lo, GEN_EXEC_BASE);
	h_eq_u64("ET_EXEC base fixe", elf_pick_base(&in, 12345), 0);
	h_eq_u64("ET_EXEC pas de creneau", elf_base_slots(&in), 0);
	h_eq_u64("ET_EXEC memoire", elf_mem_bytes(&in), 0x4000);
}

static void	hdr_sizes(void)
{
	t_gen		g;
	t_elfinfo	in;

	gen_valid(&g, ELF_ET_DYN);
	h_eq_i64("image nulle", elf_validate(NULL, GEN_SIZE, &in), E_INVAL);
	h_eq_i64("sortie nulle", elf_validate(g.buf, GEN_SIZE, NULL), E_INVAL);
	h_eq_i64("taille trop grande",
		elf_validate(g.buf, ELF_FILE_MAX + 1, &in), E_INVAL);
	g.size = 0;
	h_eq_i64("taille 0", gen_check(&g, &in), E_INVAL);
	g.size = 63;
	h_eq_i64("taille 63", gen_check(&g, &in), E_INVAL);
	g.size = 64;
	h_eq_i64("taille 64 sans table", gen_check(&g, &in), E_INVAL);
	g.size = GEN_SIZE - 1;
	h_eq_i64("taille fichier - 1", gen_check(&g, &in), E_INVAL);
}

static void	hdr_fields(void)
{
	gen_run(g_hdr, sizeof(g_hdr) / sizeof(g_hdr[0]), ELF_ET_DYN);
}

int	main(void)
{
	h_begin("a07/elf_entete");
	h_run("ET_DYN valide", hdr_valid_dyn);
	h_run("ET_EXEC valide", hdr_valid_exec);
	h_run("tailles", hdr_sizes);
	h_run("champs de l'en-tete", hdr_fields);
	return (h_end());
}
