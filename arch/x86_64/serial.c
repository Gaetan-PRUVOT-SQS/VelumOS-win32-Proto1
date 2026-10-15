#include "velum/arch.h"
#include "velum/io.h"

#define COM1 0x3f8

void	serial_init(void)
{
	outb(COM1 + 1, 0x00);
	outb(COM1 + 3, 0x80);
	outb(COM1, 0x01);
	outb(COM1 + 1, 0x00);
	outb(COM1 + 3, 0x03);
	outb(COM1 + 2, 0xc7);
	outb(COM1 + 4, 0x0b);
}

static void	serial_putc(char c)
{
	int	spin;

	spin = 200000;
	while (!(inb(COM1 + 5) & 0x20) && spin > 0)
		spin--;
	outb(COM1, (uint8_t)c);
}

void	serial_write(const char *s, size_t n)
{
	while (n--)
	{
		if (*s == '\n')
			serial_putc('\r');
		serial_putc(*s++);
	}
}

void	qemu_exit(uint8_t code)
{
	outb(0xf4, code);
}

_Noreturn void	arch_halt_forever(void)
{
	while (1)
		__asm__ volatile ("cli; hlt");
}
