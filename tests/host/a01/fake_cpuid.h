#ifndef FAKE_CPUID_H
# define FAKE_CPUID_H

# include <stdint.h>

# define FAKE_LEAVES 16
# define FAKE_CALLS 64
# define FAKE_GARBAGE 0xdeadbeefu

typedef struct s_fakecpu
{
	uint32_t	leaf[FAKE_LEAVES];
	uint32_t	regs[FAKE_LEAVES][4];
	int			n;
	uint32_t	calls[FAKE_CALLS];
	int			ncalls;
}	t_fakecpu;

void	fake_reset(void);
void	fake_set(uint32_t leaf, const uint32_t regs[4]);
void	fake_cpuid(uint32_t leaf, uint32_t sub, uint32_t out[4]);
int		fake_called(uint32_t leaf);
void	fake_brand(const char *brand);

#endif
