#ifndef A07_FAKE_H
# define A07_FAKE_H

# include <stdint.h>
# include "proc_elf.h"
# include "proc_pure.h"
# include "velum/cpu.h"
# include "velum/ksyscall.h"

# define GEN_SIZE 0x3000
# define GEN_PHNUM 6
# define GEN_ENTRY 0x800
# define GEN_DYN_OFF 0x2000
# define GEN_RELA_OFF 0x2100
# define GEN_TARGET 0x2200
# define GEN_EXEC_BASE 0x400000
# define GEN_PH_TEXT 0
# define GEN_PH_RO 1
# define GEN_PH_RW 2
# define GEN_PH_DYN 3
# define GEN_PH_RELRO 4
# define GEN_PH_STACK 5
# define EH_TYPE 16
# define EH_MACHINE 18
# define EH_VERSION 20
# define EH_ENTRY 24
# define EH_PHOFF 32
# define EH_EHSIZE 52
# define EH_PHENTSIZE 54
# define EH_PHNUM 56
# define PH0 64
# define PH1 120
# define PH2 176
# define PH3 232
# define PH4 288
# define PH5 344
# define P_TYPE 0
# define P_FLAGS 4
# define P_OFFSET 8
# define P_VADDR 16
# define P_FILESZ 32
# define P_MEMSZ 40
# define P_ALIGN 48
# define DYN0 0x2000
# define DYN1 0x2010
# define DYN2 0x2020
# define DYN3 0x2030
# define DYN4 0x2040
# define RELA0 0x2100
# define RELA1 0x2118
# define R_INFO 8
# define PATCH_MAX 3
# define UST_MAX 8
# define UST_TOP 0x7fff0000ull

typedef struct s_gen
{
	uint8_t		buf[GEN_SIZE];
	uint64_t	size;
}	t_gen;

typedef struct s_patch
{
	const char	*name;
	int			want;
	uint32_t	off[PATCH_MAX];
	uint32_t	len[PATCH_MAX];
	uint64_t	val[PATCH_MAX];
}	t_patch;

typedef struct s_fuzz
{
	uint64_t	rng;
	int			rejected;
	int			accepted;
}	t_fuzz;

typedef struct s_ustparsed
{
	uint64_t	argc;
	uint64_t	argv[UST_MAX];
	uint64_t	aux_type[UST_MAX];
	uint64_t	aux_val[UST_MAX];
	int			naux;
	int			ok;
}	t_ustparsed;

typedef struct s_fakelog
{
	int	lines;
}	t_fakelog;

extern t_fakelog		g_fakelog;
extern t_fuzz			g_fz;
extern const uint8_t	g_ust_rnd[USTACK_RANDOM];

void		gen_valid(t_gen *g, uint16_t type);
void		gen_get_eh(const t_gen *g, t_elf64_ehdr *eh);
void		gen_set_eh(t_gen *g, const t_elf64_ehdr *eh);
void		gen_get_ph(const t_gen *g, int i, t_elf64_phdr *ph);
void		gen_set_ph(t_gen *g, int i, const t_elf64_phdr *ph);
void		gen_set_dyn(t_gen *g, int i, int64_t tag, uint64_t val);
void		gen_set_rela(t_gen *g, int i, uint64_t off, uint64_t info);
int			gen_check(t_gen *g, t_elfinfo *out);
void		gen_patch(t_gen *g, uint32_t off, uint32_t len, uint64_t val);
void		gen_run(const t_patch *t, uint32_t n, uint16_t type);
uint64_t	fz_next(void);
int			fz_invariants(const t_elfinfo *in, uint64_t size);
void		ust_parse(const t_ustack *st, t_ustparsed *out);
const char	*ust_str(const t_ustack *st, uint64_t va);
int			ust_build(t_ustack *st, uint64_t cap, const t_ustart *in);
void		ust_input(t_ustart *in, const char *args, uint64_t len, int n);
int			ust_aux_order_ok(const t_ustparsed *p);
char		*args_block(int n, uint64_t each);
void		regs_user(t_regs *r);
int64_t		sys_fake_echo(const t_sysargs *a);
int64_t		sys_fake_other(const t_sysargs *a);

#endif
