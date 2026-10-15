#ifndef FAKES_H
# define FAKES_H

# include <setjmp.h>
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include <string.h>
# include "dispi.h"
# include "harness.h"
# include "velum/cpu.h"
# include "velum/display.h"
# include "velum/font.h"
# include "velum/gfx.h"
# include "velum/ksyscall.h"
# include "velum/proc.h"

# define FAKE_LOG 512
# define FAKE_WIN 16
# define FAKE_MAP 16
# define FAKE_GUARD 64
# define FAKE_GUARD_WORD 0xdeadbeef
# define FAKE_BAD_PTR 1
# define FAKE_FB_PHYS 0xe0000000ull
# define FAKE_DISPI_LOG 64

typedef struct s_fdraw
{
	uint32_t	cp;
	int32_t		x;
	int32_t		y;
	t_color		color;
}	t_fdraw;

typedef struct s_ffont
{
	t_font		font;
	t_fdraw		log[FAKE_LOG];
	uint32_t	calls;
	uint32_t	glyphs;
	int32_t		cell_w;
	int			utf8_bug;
	bool		null_font;
}	t_ffont;

typedef struct s_fluna
{
	uint32_t	calls;
	uint32_t	pct;
	uint32_t	tick;
	int32_t		w;
	int32_t		h;
}	t_fluna;

typedef struct s_fk
{
	char		last[160];
	int			info;
	int			warn;
	int			err;
	int			sinks;
	int			screens;
	int			irq_depth;
	int			halts;
	void		(*sink)(const char *s, size_t n);
	void		(*screen)(const char *t, const char *m);
	jmp_buf		halt_env;
	t_cpufeat	cpu;
}	t_fk;

typedef struct s_fwin
{
	uint8_t		*raw;
	uint8_t		*p;
	uint64_t	phys;
	size_t		len;
	uint32_t	flags;
	bool		live;
}	t_fwin;

typedef struct s_fmap
{
	t_aspace	*as;
	uint64_t	va;
	uint64_t	pa;
	uint64_t	len;
	uint32_t	flags;
	bool		live;
}	t_fmap;

typedef struct s_fvmm
{
	t_fwin		win[FAKE_WIN];
	t_fmap		map[FAKE_MAP];
	uint32_t	fail_io_mask;
	uint32_t	fail_map_mask;
	uint32_t	fail_find_mask;
	uint32_t	io_calls;
	uint32_t	map_calls;
	uint32_t	find_calls;
	uint32_t	last_io_flags;
	int			io_maps;
	int			io_unmaps;
	int			maps;
	int			unmaps;
	int			bad_unmaps;
	bool		guard_broken;
}	t_fvmm;

typedef struct s_fp
{
	t_process	proc;
	t_process	*cur;
	t_sysfn		sys[256];
	uint64_t	seed;
	uint64_t	last_bound;
	uint64_t	ok_base;
	uint64_t	ok_len;
	uint8_t		as_tag[2];
	int			locks;
	int			unlocks;
	int			bad_locks;
	int			randoms;
}	t_fp;

typedef struct s_fd
{
	uint16_t	regs[11];
	uint16_t	log[FAKE_DISPI_LOG][2];
	uint32_t	nlog;
	uint32_t	vram64k;
	uint32_t	id_reply;
	uint32_t	reject_w;
	uint32_t	virt_override;
	uint64_t	bar_bytes;
	int			open_calls;
	bool		present;
	bool		reject_enable;
	bool		clamp_height;
}	t_fd;

typedef struct s_gsurf
{
	uint32_t	*base;
	uint32_t	*px;
	size_t		count;
	t_surface	s;
}	t_gsurf;

extern t_ffont	g_ffont;
extern t_fluna	g_fl;
extern t_fk		g_fk;
extern t_fvmm	g_fvmm;
extern t_fp		g_fp;
extern t_fd		g_fd;

void		fake_reset(void);
void		fake_font_reset(void);
void		fake_vmm_reset(void);
void		fake_proc_reset(void);
void		fake_dispi_reset(void);
uint16_t	fake_dispi_read(void *ctx, uint16_t idx);
void		fake_dispi_write(void *ctx, uint16_t idx, uint16_t val);
void		fake_dispi_bind(t_dispi *d);
void		fake_dispi_ready(t_dispi *d);
bool		fd_wrote(uint32_t i, uint16_t idx, uint16_t val);
void		fake_regs_fill(uint16_t *regs, size_t n, uint16_t v);
int			fake_regs_foreign(const uint16_t *regs, size_t n, uint16_t v);
void		fake_fb_set(uint32_t w, uint32_t h);
void		fake_cmdline(const char *s);
bool		fake_gsurf_new(t_gsurf *g, int32_t w, int32_t h, int32_t stride);
bool		fake_gsurf_ok(const t_gsurf *g);
void		fake_gsurf_free(t_gsurf *g);
uint32_t	fake_count(const t_surface *s, t_rect r, t_color c);
bool		fake_vmm_guards_ok(void);
int			fake_vmm_live_windows(void);
int			fake_vmm_live_maps(void);
t_fmap		*fake_vmm_overlap(t_aspace *as, uint64_t va, uint64_t len);
int64_t		fake_sys(uint32_t num, uint64_t a0, uint64_t a1, uint64_t a2);
void		fake_proc_set(uint32_t pid, uint32_t flags);

#endif
