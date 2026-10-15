#include <string.h>
#include "a07_fake.h"

static void	gen_header(t_gen *g, uint16_t type, uint64_t base)
{
	t_elf64_ehdr	eh;

	memset(&eh, 0, sizeof(eh));
	memcpy(eh.ident, "\177ELF\2\1\1", 7);
	eh.type = type;
	eh.machine = ELF_EM_X86_64;
	eh.version = 1;
	eh.entry = base + GEN_ENTRY;
	eh.phoff = sizeof(eh);
	eh.ehsize = sizeof(eh);
	eh.phentsize = sizeof(t_elf64_phdr);
	eh.phnum = GEN_PHNUM;
	gen_set_eh(g, &eh);
}

static void	gen_ph(t_gen *g, int i, const uint64_t v[4], uint32_t type)
{
	t_elf64_phdr	ph;

	memset(&ph, 0, sizeof(ph));
	ph.type = type;
	ph.flags = (uint32_t)v[3];
	ph.offset = v[0];
	ph.vaddr = v[1];
	ph.paddr = v[1];
	ph.filesz = v[2];
	ph.memsz = v[2];
	ph.align = ELF_PAGE;
	gen_set_ph(g, i, &ph);
}

static void	gen_segments(t_gen *g, uint64_t b)
{
	t_elf64_phdr	ph;
	const uint64_t	text[4] = {0, b, 0x1000, ELF_PF_R | ELF_PF_X};
	const uint64_t	ro[4] = {0x1000, b + 0x1000, 0x1000, ELF_PF_R};
	const uint64_t	rw[4] = {0x2000, b + 0x2000, 0x1000, ELF_PF_R | ELF_PF_W};
	const uint64_t	dyn[4] = {0x2000, b + 0x2000, 0x40, ELF_PF_R | ELF_PF_W};

	gen_ph(g, GEN_PH_TEXT, text, ELF_PT_LOAD);
	gen_ph(g, GEN_PH_RO, ro, ELF_PT_LOAD);
	gen_ph(g, GEN_PH_RW, rw, ELF_PT_LOAD);
	gen_get_ph(g, GEN_PH_RW, &ph);
	ph.memsz = 0x1800;
	gen_set_ph(g, GEN_PH_RW, &ph);
	gen_ph(g, GEN_PH_DYN, dyn, ELF_PT_DYNAMIC);
	gen_ph(g, GEN_PH_RELRO, rw, ELF_PT_GNU_RELRO);
	gen_get_ph(g, GEN_PH_STACK, &ph);
	ph.type = ELF_PT_GNU_STACK;
	ph.flags = ELF_PF_R | ELF_PF_W;
	gen_set_ph(g, GEN_PH_STACK, &ph);
}

void	gen_valid(t_gen *g, uint16_t type)
{
	uint64_t	b;

	memset(g, 0, sizeof(*g));
	g->size = GEN_SIZE;
	b = 0;
	if (type == ELF_ET_EXEC)
		b = GEN_EXEC_BASE;
	gen_header(g, type, b);
	gen_segments(g, b);
	gen_set_dyn(g, 0, ELF_DT_RELA, b + GEN_RELA_OFF);
	gen_set_dyn(g, 1, ELF_DT_RELASZ, 2 * sizeof(t_elf64_rela));
	gen_set_dyn(g, 2, ELF_DT_RELAENT, sizeof(t_elf64_rela));
	gen_set_dyn(g, 3, ELF_DT_NULL, 0);
	gen_set_rela(g, 0, b + GEN_TARGET, ELF_R_RELATIVE);
	gen_set_rela(g, 1, b + GEN_TARGET + 8, ELF_R_RELATIVE);
	g->buf[GEN_ENTRY] = 0x90;
}
