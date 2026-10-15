#ifndef MSR_H
# define MSR_H

# include <stdint.h>

# define MSR_EFER 0xc0000080
# define MSR_STAR 0xc0000081
# define MSR_LSTAR 0xc0000082
# define MSR_SFMASK 0xc0000084
# define MSR_FS_BASE 0xc0000100
# define MSR_GS_BASE 0xc0000101
# define MSR_KERNEL_GS_BASE 0xc0000102
# define MSR_APIC_BASE 0x1b
# define MSR_PAT 0x277

static inline uint64_t	msr_read(uint32_t msr)
{
	uint32_t	lo;
	uint32_t	hi;

	__asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	return (((uint64_t)hi << 32) | lo);
}

static inline void	msr_write(uint32_t msr, uint64_t v)
{
	uint32_t	lo;
	uint32_t	hi;

	lo = (uint32_t)v;
	hi = (uint32_t)(v >> 32);
	__asm__ volatile ("wrmsr" : : "a"(lo), "d"(hi), "c"(msr) : "memory");
}

#endif
