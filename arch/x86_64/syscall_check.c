#include "syscall_int.h"

uint64_t	syscall_user_rflags(uint64_t rflags)
{
	return ((rflags & RFLAGS_USER_MASK) | RFLAGS_IF | RFLAGS_FIXED);
}

int	sysret_ok(const t_regs *r)
{
	if (r->cs != GDT_UCODE || r->ss != GDT_UDATA)
		return (0);
	if (r->rip >= USER_CANON_END || r->rsp >= USER_CANON_END)
		return (0);
	if (r->rcx != r->rip || r->r11 != r->rflags)
		return (0);
	if (r->rflags != syscall_user_rflags(r->rflags))
		return (0);
	return (1);
}

int	syscall_prepare_return(t_regs *r)
{
	r->rflags = syscall_user_rflags(r->rflags);
	r->rcx = r->rip;
	r->r11 = r->rflags;
	if (r->rip >= USER_CANON_END)
		return (KILL_PATH);
	if (sysret_ok(r))
		return (SYSRET_PATH);
	return (IRET_PATH);
}
