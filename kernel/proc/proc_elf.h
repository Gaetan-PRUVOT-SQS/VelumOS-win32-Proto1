#ifndef PROC_ELF_H
# define PROC_ELF_H

# include <stdint.h>

# define ELF_SEG_MAX 16
# define ELF_PHNUM_MAX 64
# define ELF_FILE_MAX 0x4000000ull
# define ELF_SPAN_MAX 0x4000000ull
# define ELF_PAGE 4096ull
# define ELF_ET_EXEC 2
# define ELF_ET_DYN 3
# define ELF_EM_X86_64 62
# define ELF_PT_LOAD 1
# define ELF_PT_DYNAMIC 2
# define ELF_PT_INTERP 3
# define ELF_PT_GNU_STACK 0x6474e551
# define ELF_PT_GNU_RELRO 0x6474e552
# define ELF_PF_X 1
# define ELF_PF_W 2
# define ELF_PF_R 4
# define ELF_DT_NULL 0
# define ELF_DT_NEEDED 1
# define ELF_DT_PLTRELSZ 2
# define ELF_DT_RELA 7
# define ELF_DT_RELASZ 8
# define ELF_DT_RELAENT 9
# define ELF_DT_REL 17
# define ELF_DT_RELSZ 18
# define ELF_DT_TEXTREL 22
# define ELF_DT_JMPREL 23
# define ELF_DT_FLAGS 30
# define ELF_DT_RELRSZ 35
# define ELF_DT_RELR 36
# define ELF_DF_TEXTREL 4
# define ELF_R_RELATIVE 8
# define ELF_ASLR_LO 0x10000000ull
# define ELF_ASLR_HI 0x00007e0000000000ull
# define ELF_ASLR_ALIGN 0x200000ull

typedef struct s_elf64_ehdr
{
	uint8_t		ident[16];
	uint16_t	type;
	uint16_t	machine;
	uint32_t	version;
	uint64_t	entry;
	uint64_t	phoff;
	uint64_t	shoff;
	uint32_t	flags;
	uint16_t	ehsize;
	uint16_t	phentsize;
	uint16_t	phnum;
	uint16_t	shentsize;
	uint16_t	shnum;
	uint16_t	shstrndx;
}	t_elf64_ehdr;

typedef struct s_elf64_phdr
{
	uint32_t	type;
	uint32_t	flags;
	uint64_t	offset;
	uint64_t	vaddr;
	uint64_t	paddr;
	uint64_t	filesz;
	uint64_t	memsz;
	uint64_t	align;
}	t_elf64_phdr;

typedef struct s_elf64_dyn
{
	int64_t		tag;
	uint64_t	val;
}	t_elf64_dyn;

typedef struct s_elf64_rela
{
	uint64_t	offset;
	uint64_t	info;
	int64_t		addend;
}	t_elf64_rela;

typedef struct s_elfimg
{
	const uint8_t	*data;
	uint64_t		size;
}	t_elfimg;

typedef struct s_elfseg
{
	uint64_t	vaddr;
	uint64_t	memsz;
	uint64_t	offset;
	uint64_t	filesz;
	uint32_t	flags;
	uint32_t	reserved;
}	t_elfseg;

typedef struct s_elfdyn
{
	uint64_t	rela;
	uint64_t	relasz;
	uint64_t	relaent;
	uint32_t	has_rela;
	uint32_t	reserved;
}	t_elfdyn;

typedef struct s_elfinfo
{
	uint32_t	type;
	uint32_t	nseg;
	uint32_t	phnum;
	uint32_t	has_dyn;
	uint64_t	entry;
	uint64_t	phoff;
	uint64_t	phdr_vaddr;
	uint64_t	span_lo;
	uint64_t	span_hi;
	uint64_t	dyn_off;
	uint64_t	dyn_size;
	uint64_t	rela_off;
	uint64_t	rela_count;
	uint64_t	relro_lo;
	uint64_t	relro_hi;
	t_elfseg	seg[ELF_SEG_MAX];
}	t_elfinfo;

int			elf_validate(const void *image, uint64_t size, t_elfinfo *out);
int			elf_read(const t_elfimg *im, uint64_t off, void *dst, uint64_t n);
int			elf_scan_phdrs(const t_elfimg *im, const t_elf64_ehdr *eh,
				t_elfinfo *out);
int			elf_check_final(t_elfinfo *out);
int			elf_check_dynamic(const t_elfimg *im, t_elfinfo *out);
int			elf_find_seg(const t_elfinfo *in, uint64_t va, uint64_t len);
uint64_t	elf_base_slots(const t_elfinfo *in);
uint64_t	elf_pick_base(const t_elfinfo *in, uint64_t slot);
uint64_t	elf_mem_bytes(const t_elfinfo *in);
int			elf_get_rela(const t_elfimg *im, const t_elfinfo *in, uint64_t i,
				t_elf64_rela *out);

#endif
