#ifndef ACPI_H
# define ACPI_H

# include <stdbool.h>
# include <stdint.h>

# define ACPI_MAX_CPUS 64
# define ACPI_MAX_IOAPICS 8
# define ACPI_MAX_OVERRIDES 16
# define ACPI_MAX_MCFG 8
# define ACPI_POL_MASK 0x3
# define ACPI_POL_LOW 0x3
# define ACPI_TRIG_MASK 0xc
# define ACPI_TRIG_LEVEL 0xc

struct __attribute__((packed)) s_acpi_sdt
{
	char		sig[4];
	uint32_t	length;
	uint8_t		revision;
	uint8_t		checksum;
	char		oem_id[6];
	char		oem_table_id[8];
	uint32_t	oem_revision;
	uint32_t	creator_id;
	uint32_t	creator_revision;
};

typedef struct s_ioapic_info
{
	uint32_t	id;
	uint32_t	gsi_base;
	uint64_t	phys;
}	t_ioapic_info;

typedef struct s_irq_override
{
	uint8_t		isa;
	uint8_t		reserved;
	uint16_t	flags;
	uint32_t	gsi;
}	t_irq_override;

typedef struct s_mcfg_entry
{
	uint64_t	base;
	uint16_t	segment;
	uint8_t		bus_start;
	uint8_t		bus_end;
	uint32_t	reserved;
}	t_mcfg_entry;

typedef struct s_acpi_info
{
	uint32_t		ncpus;
	uint32_t		lapic_id[ACPI_MAX_CPUS];
	uint64_t		lapic_phys;
	uint32_t		nioapics;
	t_ioapic_info	ioapic[ACPI_MAX_IOAPICS];
	uint32_t		noverrides;
	t_irq_override	override[ACPI_MAX_OVERRIDES];
	uint32_t		nmcfg;
	t_mcfg_entry	mcfg[ACPI_MAX_MCFG];
	uint64_t		hpet_phys;
	uint16_t		sci_irq;
	uint16_t		pm1a_cnt;
	uint16_t		pm1b_cnt;
	uint16_t		slp_typ_a;
	uint16_t		slp_typ_b;
	bool			poweroff_ok;
	uint8_t			century_reg;
	uint8_t			reset_value;
	uint16_t		reset_port;
}	t_acpi_info;

int						acpi_boot_init(void);
const t_acpi_info		*acpi_info(void);
const struct s_acpi_sdt	*acpi_find_table(const char *sig, uint32_t index);

#endif
