#ifndef ACPI_INT_H
# define ACPI_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/acpi.h"

# define ACPI_HDR_LEN 36
# define ACPI_TABLE_MAX_LEN 0x400000
# define ACPI_TABLES_MAX 64
# define ACPI_MAX_NMI 8
# define ACPI_UID_ALL 0xffffffffu
# define ACPI_RSDP_V1_LEN 20
# define ACPI_RSDP_V2_LEN 36
# define ACPI_RSDP_MAX_LEN 4096
# define RSDP_REVISION 15
# define RSDP_RSDT 16
# define RSDP_LENGTH 20
# define RSDP_XSDT 24
# define MADT_LAPIC_ADDR 36
# define MADT_FLAGS 40
# define MADT_ENTRIES 44
# define MADT_PCAT_COMPAT 0x1
# define MADT_LAPIC_ENABLED 0x1
# define FADT_MIN_LEN 116
# define FADT_DSDT 40
# define FADT_PROFILE 45
# define FADT_SCI_INT 46
# define FADT_SMI_CMD 48
# define FADT_ACPI_ENABLE 52
# define FADT_PM1A_CNT 64
# define FADT_PM1B_CNT 68
# define FADT_PM1_CNT_LEN 89
# define FADT_CENTURY 108
# define FADT_BOOT_ARCH 109
# define FADT_FLAGS 112
# define FADT_RESET_REG 116
# define FADT_RESET_VALUE 128
# define FADT_X_DSDT 140
# define FADT_X_PM1A_CNT 172
# define FADT_X_PM1B_CNT 184
# define FADT_RESET_SUP 0x400
# define FADT_HW_REDUCED 0x100000
# define GAS_LEN 12
# define GAS_SPACE_IO 1
# define GAS_SPACE_MEM 0
# define GAS_ADDR 4
# define HPET_TABLE_ADDR 40
# define HPET_TABLE_MIN 56
# define MCFG_ENTRIES 44
# define MCFG_ENTRY_LEN 16
# define AML_NAME_OP 0x08
# define AML_ROOT_CHAR 0x5c
# define AML_PACKAGE_OP 0x12
# define AML_ZERO_OP 0x00
# define AML_ONE_OP 0x01
# define AML_BYTE_PREFIX 0x0a
# define AML_WORD_PREFIX 0x0b
# define AML_DWORD_PREFIX 0x0c
# define AML_QWORD_PREFIX 0x0e
# define SLP_TYP_MAX 7
# define CMOS_FIRST_FREE 0x0e
# define CMOS_LAST_INDEX 0x7f
# define ACPI_PHYS_LIMIT 0x0010000000000000ull

typedef const void	*(*t_acpi_mapfn)(uint64_t phys, uint64_t len, void *ctx);
typedef void		(*t_acpi_unmapfn)(const void *v, uint64_t len, void *ctx);

typedef struct s_acpi_src
{
	t_acpi_mapfn	map;
	t_acpi_unmapfn	unmap;
	void			*ctx;
}	t_acpi_src;

typedef struct s_acpi_root
{
	uint64_t	phys;
	uint32_t	entry_size;
	uint32_t	revision;
}	t_acpi_root;

typedef struct s_acpi_tab
{
	const struct s_acpi_sdt	*hdr;
	uint64_t				phys;
}	t_acpi_tab;

typedef struct s_acpi_set
{
	t_acpi_tab	tab[ACPI_TABLES_MAX];
	uint32_t	count;
	uint32_t	rejected;
	uint32_t	dropped;
}	t_acpi_set;

typedef struct s_acpi_nmi
{
	uint32_t	uid;
	uint16_t	flags;
	uint8_t		lint;
	uint8_t		reserved;
}	t_acpi_nmi;

typedef struct s_acpi_extra
{
	uint32_t	lapic_uid[ACPI_MAX_CPUS];
	t_acpi_nmi	nmi[ACPI_MAX_NMI];
	uint32_t	nnmi;
	uint32_t	nmi_src_gsi[ACPI_MAX_NMI];
	uint32_t	nnmi_src;
	uint32_t	madt_flags;
	uint32_t	madt_skipped;
	uint32_t	cpus_ignored;
	uint64_t	lapic_ovr;
	uint32_t	fadt_flags;
	uint16_t	smi_cmd;
	uint8_t		acpi_enable;
	uint8_t		pm_profile;
	uint16_t	boot_arch;
	uint8_t		pm1_cnt_len;
	bool		have_madt;
	bool		have_fadt;
	bool		have_s5;
}	t_acpi_extra;

bool					acpi_sig_eq(const void *a, const char *sig);
typedef struct s_acpi_state
{
	t_acpi_info		info;
	t_acpi_extra	x;
	t_acpi_set		set;
	t_acpi_root		root;
}	t_acpi_state;

uint64_t				acpi_rd(const void *base, uint32_t off, uint32_t size);
uint8_t					acpi_sum(const void *p, uint64_t len);
int						acpi_rsdp_parse(const t_acpi_src *s, uint64_t phys,
							t_acpi_root *out);
const struct s_acpi_sdt	*acpi_map_table(const t_acpi_src *s, uint64_t phys,
							int *err);
int						acpi_set_add(t_acpi_set *set,
							const struct s_acpi_sdt *t, uint64_t phys);
int						acpi_collect(const t_acpi_src *s,
							const t_acpi_root *root, t_acpi_set *set);
const struct s_acpi_sdt	*acpi_set_find(const t_acpi_set *set,
							const char *sig, uint32_t index);
int						acpi_parse_madt(const struct s_acpi_sdt *t,
							t_acpi_info *i, t_acpi_extra *x);
void					madt_entry(const uint8_t *e, uint8_t len,
							t_acpi_info *i, t_acpi_extra *x);
void					madt_iso(const uint8_t *e, uint8_t len,
							t_acpi_info *i, t_acpi_extra *x);
void					madt_nmi(const uint8_t *e, uint8_t len,
							t_acpi_info *i, t_acpi_extra *x);
void					madt_add_cpu(uint32_t apic_id, uint32_t uid,
							t_acpi_info *i, t_acpi_extra *x);
void					madt_nmi_src(const uint8_t *e, uint8_t len,
							t_acpi_info *i, t_acpi_extra *x);
void					madt_lapic_ovr(const uint8_t *e, uint8_t len,
							t_acpi_info *i, t_acpi_extra *x);
bool					fadt_get(const struct s_acpi_sdt *t, uint32_t off,
							uint32_t size, uint64_t *out);
void					fadt_basic(const struct s_acpi_sdt *t,
							t_acpi_info *i, t_acpi_extra *x);
int						acpi_parse_fadt(const struct s_acpi_sdt *t,
							t_acpi_info *i, t_acpi_extra *x);
uint64_t				acpi_fadt_dsdt(const struct s_acpi_sdt *t);
int						acpi_find_s5(const uint8_t *aml, uint64_t len,
							uint16_t *typ_a, uint16_t *typ_b);
int						acpi_parse_hpet(const struct s_acpi_sdt *t,
							t_acpi_info *i);
int						acpi_parse_mcfg(const struct s_acpi_sdt *t,
							t_acpi_info *i);
int						acpi_parse_all(const t_acpi_set *set,
							t_acpi_info *i, t_acpi_extra *x);
const t_acpi_extra		*acpi_extra(void);
t_acpi_state			*acpi_state(void);
void					acpi_log(const t_acpi_state *st);

#endif
