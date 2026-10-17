#ifndef BOOT_INT_H
# define BOOT_INT_H

# include "velum/boot.h"

int		boot_fill_memmap(t_bootinfo *bi);
void	boot_fill_misc(t_bootinfo *bi);
uint64_t	boot_guard_seed(void);
const char	*boot_guard_source(void);

#endif
