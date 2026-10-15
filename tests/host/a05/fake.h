#ifndef FAKE_H
# define FAKE_H

# include <stdbool.h>
# include <stdint.h>
# include "acpi_int.h"
# include "irq_int.h"

# define FAKE_BASE 0xe0000ull
# define FAKE_SIZE 0x40000ull
# define F_RSDP 0xe0000ull
# define F_RSDT 0xe0800ull
# define F_XSDT 0xe1000ull
# define F_FACP 0xe2000ull
# define F_APIC 0xe3000ull
# define F_HPET 0xe4000ull
# define F_MCFG 0xe5000ull
# define F_SSDT 0xe6000ull
# define F_DSDT 0xe7000ull
# define F_SSDT2 0xe8000ull

typedef struct s_fakemem
{
	uint8_t		mem[FAKE_SIZE];
	uint64_t	maps;
	uint64_t	unmaps;
	int64_t		fail_after;
}	t_fakemem;

typedef struct s_fakeclock
{
	uint64_t	now;
	uint64_t	step;
	uint64_t	relax;
	uint64_t	delays;
	bool		ready;
}	t_fakeclock;

extern t_fakemem	g_fmem;
extern t_fakeclock	g_fclock;

void		facpi_reset(void);
uint8_t		*facpi_at(uint64_t phys);
void		facpi_put(uint64_t phys, uint32_t off, uint64_t v, uint32_t size);
void		facpi_hdr(uint64_t phys, const char *sig, uint32_t len,
				uint8_t rev);
void		facpi_fix(uint64_t phys);
t_acpi_src	facpi_src(void);
void		facpi_rsdp(uint8_t rev, uint64_t rsdt, uint64_t xsdt);
void		facpi_root(uint64_t phys, uint32_t esize, const uint64_t *tabs,
				uint32_t n);
void		facpi_fadt(uint32_t len, uint64_t dsdt);
void		facpi_aml(uint64_t phys, const char *sig, const uint8_t *aml,
				uint32_t n);
void		facpi_std(uint32_t esize);
void		facpi_madt(const uint8_t *entries, uint32_t n);
void		ffn_a(void *ctx);
void		ffn_b(void *ctx);
void		ffn_c(void *ctx);
int			firq_req(t_irq_core *c, uint32_t gsi, t_irqfn fn, uint32_t flags);
void		fclock_reset(bool ready, uint64_t step);

#endif
