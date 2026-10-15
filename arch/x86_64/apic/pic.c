#include "apic_int.h"
#include "velum/cpu.h"
#include "velum/io.h"

static void	io_wait(void)
{
	outb(IO_DELAY_PORT, 0);
}

static void	pic_spurious(t_regs *regs, void *ctx)
{
	(void)regs;
	(void)ctx;
}

static void	pic_write(uint16_t port, uint8_t val)
{
	outb(port, val);
	io_wait();
}

void	pic_disable(void)
{
	pic_write(PIC1_CMD, PIC_ICW1);
	pic_write(PIC2_CMD, PIC_ICW1);
	pic_write(PIC1_DATA, PIC_VEC_MASTER);
	pic_write(PIC2_DATA, PIC_VEC_SLAVE);
	pic_write(PIC1_DATA, 4);
	pic_write(PIC2_DATA, 2);
	pic_write(PIC1_DATA, PIC_ICW4);
	pic_write(PIC2_DATA, PIC_ICW4);
	pic_write(PIC1_DATA, 0xff);
	pic_write(PIC2_DATA, 0xff);
	idt_set_handler(PIC_SPURIOUS_MASTER, pic_spurious, NULL);
	idt_set_handler(PIC_SPURIOUS_SLAVE, pic_spurious, NULL);
}
