#ifndef APIC_INT_H
# define APIC_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/acpi.h"

# define LAPIC_ID 0x20
# define LAPIC_TPR 0x80
# define LAPIC_EOI 0xb0
# define LAPIC_SVR 0xf0
# define LAPIC_ESR 0x280
# define LAPIC_ICR_LO 0x300
# define LAPIC_LVT_TIMER 0x320
# define LAPIC_LVT_LINT0 0x350
# define LAPIC_LVT_LINT1 0x360
# define LAPIC_LVT_ERROR 0x370
# define LAPIC_TIMER_INIT 0x380
# define LAPIC_TIMER_CUR 0x390
# define LAPIC_TIMER_DIV 0x3e0
# define LAPIC_SVR_ENABLE 0x100
# define LAPIC_LVT_MASKED 0x10000
# define LAPIC_LVT_NMI 0x400
# define LAPIC_LVT_LOW 0x2000
# define LAPIC_TIMER_TSCDL 0x40000
# define LAPIC_DIV_16 0x3
# define LAPIC_ICR_SELF 0x40000
# define LAPIC_ICR_PENDING 0x1000
# define LAPIC_MMIO_LEN 0x1000
# define X2APIC_MSR_BASE 0x800
# define X2APIC_SELF_IPI 0x83f
# define MSR_TSC_DEADLINE 0x6e0
# define APIC_BASE_EXTD 0x400
# define APIC_BASE_EN 0x800
# define APIC_BASE_ADDR_MASK 0x000ffffffffff000ull
# define IOAPIC_WIN 4
# define IOAPIC_MMIO_LEN 0x20
# define IOAPIC_REG_VER 0x01
# define IOAPIC_REG_RED 0x10
# define IOAPIC_MAX_PINS 240
# define IOAPIC_RED_LOW 0x2000
# define IOAPIC_RED_LEVEL 0x8000
# define IOAPIC_RED_MASKED 0x10000
# define IOAPIC_DEST_SHIFT 56
# define PIC1_CMD 0x20
# define PIC1_DATA 0x21
# define PIC2_CMD 0xa0
# define PIC2_DATA 0xa1
# define PIC_ICW1 0x11
# define PIC_ICW4 0x01
# define PIC_VEC_MASTER 0x20
# define PIC_VEC_SLAVE 0x28
# define PIC_SPURIOUS_MASTER 0x27
# define PIC_SPURIOUS_SLAVE 0x2f
# define IO_DELAY_PORT 0x80

typedef struct s_ioapic_dev
{
	volatile uint32_t	*base;
	uint32_t			id;
	uint32_t			gsi_base;
	uint32_t			count;
	uint32_t			reserved;
}	t_ioapic_dev;

typedef struct s_ioapic_set
{
	t_ioapic_dev	dev[ACPI_MAX_IOAPICS];
	uint32_t		count;
}	t_ioapic_set;

typedef struct s_lapic_state
{
	volatile uint32_t	*mmio;
	bool				x2;
	bool				ready;
}	t_lapic_state;

uint32_t		lapic_read(uint32_t reg);
void			lapic_write(uint32_t reg, uint32_t val);
bool			lapic_ready(void);
bool			lapic_is_x2(void);
int				lapic_init(void);
void			lapic_set_mode(bool x2, volatile uint32_t *mmio);
void			lapic_nmi_setup(void);
uint32_t		lapic_lvt_nmi(uint16_t mps_flags);
int				lapic_self_ipi(uint8_t vec);
uint64_t		ioapic_entry(uint8_t vec, uint32_t trig, uint32_t dest,
					bool masked);
uint32_t		ioapic_count_from_ver(uint32_t ver);
int				ioapic_init_all(const t_acpi_info *ai);
bool			ioapic_covers(uint32_t gsi);
int				ioapic_route(uint32_t gsi, uint8_t vec, uint32_t trig,
					uint32_t dest);
int				ioapic_set_mask(uint32_t gsi, bool masked);
uint32_t		ioapic_rd(const t_ioapic_dev *d, uint32_t reg);
void			ioapic_wr(const t_ioapic_dev *d, uint32_t reg, uint32_t val);
t_ioapic_dev	*ioapic_find(uint32_t gsi, uint32_t *pin);
void			pic_disable(void);
uint32_t		ioapic_gsi_end(void);
void			lapic_timer_deadline_mode(void);
void			lapic_timer_oneshot_mode(void);
void			lapic_timer_set_count(uint32_t count);
void			lapic_timer_set_deadline(uint64_t tsc);
uint64_t		lapic_timer_calibrate(void);

#endif
