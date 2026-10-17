#ifndef STACK_REPORT_H
# define STACK_REPORT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/cpu.h"

# define STACK_GUARD_SIZE 4096ull
# define STACK_CHAIN_MAX 8
# define STACK_CHAIN_STEPS 1024
# define STACK_REPORT_SIZE 2048
# define STACK_SEL_KERNEL 0x08
# define STACK_SEL_USER 0x23

typedef enum e_stackpos
{
	SPOS_UNKNOWN,
	SPOS_INSIDE,
	SPOS_GUARD,
	SPOS_BELOW,
	SPOS_OUTSIDE
}	t_stackpos;

typedef struct s_stackinfo
{
	bool		has_thread;
	uint32_t	tid;
	const char	*name;
	uint64_t	base;
	uint64_t	top;
	uint64_t	rsp;
	uint64_t	cr2;
	uint64_t	tss_rsp0;
	uint64_t	gs_kstack;
	uint32_t	irq_depth;
	uint32_t	preempt;
}	t_stackinfo;

typedef struct s_trapframe
{
	uint64_t	vec;
	uint64_t	err;
	uint64_t	rip;
	uint64_t	cs;
	uint64_t	rsp;
}	t_trapframe;

typedef struct s_trapchain
{
	t_trapframe	head[STACK_CHAIN_MAX];
	t_trapframe	tail[STACK_CHAIN_MAX];
	int			nhead;
	int			ntail;
	int			total;
}	t_trapchain;

t_stackpos	stack_position(uint64_t addr, uint64_t base, uint64_t top);
const char	*stack_pos_name(t_stackpos pos);
uint64_t	stack_free_bytes(uint64_t rsp, uint64_t base, uint64_t top);
bool		stack_coherent(const t_stackinfo *si);
int			stack_format(char *out, size_t size, const t_stackinfo *si);
void		stack_trap_chain(t_trapchain *out, uint64_t rbp, uint64_t lo,
				uint64_t hi);
int			stack_chain_format(char *out, size_t size, const t_trapchain *ch);

#endif
