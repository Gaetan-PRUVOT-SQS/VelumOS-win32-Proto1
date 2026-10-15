#include <string.h>
#include "a07_fake.h"
#include "velum/ksyscall.h"

void	regs_user(t_regs *r)
{
	memset(r, 0, sizeof(*r));
	r->cs = GDT_UCODE;
	r->ss = GDT_UDATA;
	r->rip = 0x10001000;
	r->rsp = 0x7ffff000;
	r->rflags = 0x202;
	r->rcx = r->rip;
	r->r11 = r->rflags;
}

int64_t	sys_fake_echo(const t_sysargs *a)
{
	return ((int64_t)(a->a[0] + a->a[5]));
}

int64_t	sys_fake_other(const t_sysargs *a)
{
	(void)a;
	return (-77);
}
