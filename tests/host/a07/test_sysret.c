#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "syscall_int.h"

static void	rflags_table(void)
{
	h_eq_u64("IOPL retire", syscall_user_rflags(0x3202), 0x202);
	h_eq_u64("NT retire", syscall_user_rflags(0x4202), 0x202);
	h_eq_u64("RF retire", syscall_user_rflags(0x10202), 0x202);
	h_eq_u64("VM retire", syscall_user_rflags(0x20202), 0x202);
	h_eq_u64("IF force", syscall_user_rflags(0), 0x202);
	h_eq_u64("CF ZF SF OF DF gardes", syscall_user_rflags(0xcc1), 0xec3);
	h_eq_u64("AC et ID gardes", syscall_user_rflags(0x240000), 0x240202);
	h_eq_u64("TF garde", syscall_user_rflags(0x100), 0x302);
}

static void	sysret_mcdc(void)
{
	t_regs	r;

	regs_user(&r);
	h_eq_i64("MC/DC cas nominal", sysret_ok(&r), 1);
	r.cs = GDT_KCODE;
	h_eq_i64("MC/DC CS noyau", sysret_ok(&r), 0);
	regs_user(&r);
	r.ss = GDT_KDATA;
	h_eq_i64("MC/DC SS noyau", sysret_ok(&r), 0);
	regs_user(&r);
	r.rip = USER_CANON_END;
	r.rcx = r.rip;
	h_eq_i64("MC/DC RIP non canonique", sysret_ok(&r), 0);
	regs_user(&r);
	r.rsp = 0xffff800000000000ull;
	h_eq_i64("MC/DC RSP non canonique", sysret_ok(&r), 0);
	regs_user(&r);
	r.rcx = 0;
	h_eq_i64("MC/DC RCX different de RIP", sysret_ok(&r), 0);
}

static void	sysret_mcdc_flags(void)
{
	t_regs	r;

	regs_user(&r);
	r.r11 = 0x3202;
	h_eq_i64("MC/DC R11 different de RFLAGS", sysret_ok(&r), 0);
	regs_user(&r);
	r.rflags = 0x3202;
	r.r11 = r.rflags;
	h_eq_i64("MC/DC RFLAGS avec IOPL", sysret_ok(&r), 0);
}

static void	prepare_paths(void)
{
	t_regs	r;

	regs_user(&r);
	r.rflags = 0x7246;
	r.rcx = 0x1234;
	h_eq_i64("chemin sysret", syscall_prepare_return(&r), SYSRET_PATH);
	h_eq_u64("RCX = RIP", r.rcx, r.rip);
	h_eq_u64("R11 = RFLAGS filtre", r.r11, 0x246);
	regs_user(&r);
	r.rip = USER_CANON_END - 1;
	h_eq_i64("limite RIP canonique", syscall_prepare_return(&r), SYSRET_PATH);
	r.rip = USER_CANON_END;
	h_eq_i64("RIP non canonique : tuer", syscall_prepare_return(&r),
		KILL_PATH);
	regs_user(&r);
	r.rsp = USER_CANON_END;
	h_eq_i64("RSP non canonique : iretq", syscall_prepare_return(&r),
		IRET_PATH);
	regs_user(&r);
	r.cs = GDT_KCODE;
	h_eq_i64("CS modifie : iretq", syscall_prepare_return(&r), IRET_PATH);
}

static void	star_selecteurs(void)
{
	uint64_t	base_sysret;
	uint64_t	base_syscall;

	base_sysret = SYSCALL_STAR >> 48;
	base_syscall = (SYSCALL_STAR >> 32) & 0xffff;
	h_eq_u64("SYSRET : SS = base + 8, RPL 3 compris", base_sysret + 8,
		GDT_UDATA);
	h_eq_u64("SYSRET 64 bits : CS = base + 16, RPL 3 compris",
		base_sysret + 16, GDT_UCODE);
	h_eq_u64("SYSCALL : CS = STAR[47:32]", base_syscall, GDT_KCODE);
	h_eq_u64("SYSCALL : SS = CS + 8", base_syscall + 8, GDT_KDATA);
	h_eq_u64("STAR : 32 bits bas nuls", SYSCALL_STAR & 0xffffffffull, 0);
}

int	main(void)
{
	h_begin("a07/retour_syscall");
	h_run("RFLAGS rendu", rflags_table);
	h_run("decision sysret", sysret_mcdc);
	h_run("decision sysret, drapeaux", sysret_mcdc_flags);
	h_run("chemin de retour", prepare_paths);
	h_run("selecteurs derives de STAR", star_selecteurs);
	return (h_end());
}
