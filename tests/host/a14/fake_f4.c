#include "fake_f4.h"

void	f4_reset(t_f4_stack *s)
{
	s->n = 0;
}

void	f4_push(t_f4_stack *s, uint64_t v)
{
	if (s->n < F4_WORDS)
	{
		s->w[s->n] = v;
		s->n++;
	}
}

void	f4_head(t_f4_stack *s, uint64_t argc, uint64_t envc)
{
	uint64_t	i;

	f4_push(s, argc);
	i = 0;
	while (i < argc)
		f4_push(s, F4_PTR_BASE + 16 * i++);
	f4_push(s, 0);
	i = 0;
	while (i < envc)
		f4_push(s, F4_PTR_BASE + 0x8000 + 16 * i++);
	f4_push(s, 0);
}

void	f4_pairs(t_f4_stack *s, uint64_t count, uint64_t type)
{
	uint64_t	i;

	i = 0;
	while (i < count)
	{
		f4_push(s, type);
		f4_push(s, 0x1000 + i);
		i++;
	}
}

void	f4_aux_full(t_f4_stack *s)
{
	f4_push(s, AT_PAGESZ);
	f4_push(s, 4096);
	f4_push(s, AT_RANDOM);
	f4_push(s, 0x7000);
	f4_push(s, AT_ENTRY);
	f4_push(s, 0x401000);
	f4_push(s, AT_PHDR);
	f4_push(s, 0x400040);
	f4_push(s, AT_PHNUM);
	f4_push(s, 6);
	f4_push(s, AT_VELUM_ABI);
	f4_push(s, 1);
	f4_push(s, AT_VELUM_FLAGS);
	f4_push(s, 0x3f);
	f4_push(s, AT_NULL);
	f4_push(s, 0);
}
