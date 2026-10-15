#ifndef ARCH_H
# define ARCH_H

# include <stddef.h>
# include <stdint.h>

void			serial_init(void);
void			serial_write(const char *s, size_t n);
void			qemu_exit(uint8_t code);
_Noreturn void	arch_halt_forever(void);

#endif
