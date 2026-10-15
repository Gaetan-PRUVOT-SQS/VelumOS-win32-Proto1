#ifndef FAKE_H
# define FAKE_H

# include <stdarg.h>
# include <stdint.h>
# include <stddef.h>
# include "velum/abi/abi_input.h"
# include "velum/irq.h"
# include "velum/ksyscall.h"
# include "velum/object.h"

# define FU_BASE 0x10000ull
# define FU_SIZE 0x10000
# define FOBJ_HANDLES 32
# define FIRQ_MAX 8
# define FKLOG_SIZE 8192
# define FSYS_MAX 256

typedef struct s_fsync
{
	int	held;
	int	errors;
}	t_fsync;

typedef struct s_ftime
{
	uint64_t	now;
	uint64_t	step;
}	t_ftime;

typedef struct s_fakeobj
{
	t_object	obj;
	int			signals;
}	t_fakeobj;

typedef struct s_fobj
{
	int			live;
	int			fail_create;
	int			fail_handle;
	t_object	*slot[FOBJ_HANDLES];
	uint32_t	rights[FOBJ_HANDLES];
}	t_fobj;

typedef struct s_fproc
{
	t_process	proc;
	int			present;
}	t_fproc;

typedef struct s_fuser
{
	uint8_t		mem[FU_SIZE];
	uint64_t	fault_addr;
	int			lock_violations;
	int			copies;
}	t_fuser;

typedef struct s_firqreg
{
	uint32_t	gsi;
	t_irqfn		fn;
	void		*ctx;
	uint32_t	flags;
}	t_firqreg;

typedef struct s_firq
{
	t_firqreg	reg[FIRQ_MAX];
	int			count;
	uint32_t	fail_gsi;
	uint32_t	gsi_of_isa[16];
}	t_firq;

typedef struct s_fklog
{
	char	buf[FKLOG_SIZE];
	size_t	len;
}	t_fklog;

typedef struct s_fsys
{
	t_sysfn	fn[FSYS_MAX];
	int		fail_next;
}	t_fsys;

extern t_fsync	g_fsync;
extern t_ftime	g_ftime;
extern t_fobj	g_fobj;
extern t_fproc	g_fproc;
extern t_fuser	g_fuser;
extern t_firq	g_firq;
extern t_fklog	g_fklog;
extern t_fsys	g_fsys;

void		fsync_reset(void);
void		th_relax(void);
void		ftime_reset(void);
void		fobj_reset(void);
int			fobj_signals(const t_object *o);
void		fproc_set(uint32_t flags);
void		fproc_clear(void);
void		fuser_reset(void);
void		firq_reset(void);
void		fklog_reset(void);
int			fklog_has(const char *s);
void		fklog_vappend(const char *fmt, va_list ap);
void		fboot_set(const char *cmdline);
void		fsys_reset(void);
int64_t		fsys_call(uint32_t num, uint64_t a0, uint64_t a1, uint64_t a2);
void		fake_all_reset(void);

#endif
