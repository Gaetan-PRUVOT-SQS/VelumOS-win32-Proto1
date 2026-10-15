#ifndef DISPLAY_INT_H
# define DISPLAY_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "dispi.h"
# include "velum/boot.h"
# include "velum/display.h"
# include "velum/gfx.h"
# include "velum/ksyscall.h"
# include "velum/proc.h"
# include "velum/sync.h"

# define DISP_DIM_MAX 8192
# define DISP_PITCH_MAX 65536
# define DISP_BYTES_MAX 268435456ull
# define DISP_PHYS_LIMIT 4503599627370496ull
# define DISP_BPP 32
# define DISP_LOCK_SPINS 4000000
# define DISP_MODES_MAX 8
# define ST_W 64
# define ST_H 32
# define UMAP_SLOTS 4
# define UMAP_RANDOM_LO 0x10000000ull
# define UMAP_RANDOM_COUNT 66060160ull
# define UMAP_RANDOM_ALIGN 0x200000ull

typedef enum e_dspstate
{
	DSP_OFF = 0,
	DSP_SPLASH,
	DSP_CONSOLE
}	t_dspstate;

typedef struct s_umap
{
	uint64_t	va;
	uint64_t	len;
	uint32_t	pid;
	uint32_t	gen;
}	t_umap;

typedef struct s_display
{
	t_dispinfo	info;
	t_surface	surf;
	t_dispi		dispi;
	t_umap		umap[UMAP_SLOTS];
	t_mutex		lock;
	uint64_t	phys;
	uint64_t	map_len;
	void		*fb;
	uint32_t	map_flags;
	uint32_t	generation;
	uint32_t	tick;
	uint32_t	pct;
	uint32_t	busy;
	t_dspstate	state;
	bool		ready;
	bool		hold_on_panic;
}	t_display;

int			fbgeom_check(const t_fbinfo *fb, t_dispinfo *out);
int			display_open(void);
void		display_surface_set(t_surface *s, void *px, const t_dispinfo *i);
bool		display_ready(void);
t_surface	*display_surface(void);
t_dspstate	display_state(void);
void		display_state_set(t_dspstate st);
bool		display_try_lock(void);
bool		display_lock_wait(void);
void		display_unlock(void);
void		display_splash(uint32_t pct);
int			display_switch(uint32_t w, uint32_t h);
int			display_priv(void);
bool		display_fault_requested(void);
void		display_panic_screen(const char *t, const char *m);
void		display_register_syscalls(void);
int64_t		display_user_map(t_process *p, uint64_t hint);
uint64_t	umap_len(void);
bool		umap_hint_ok(uint64_t hint, uint64_t len);
int			umap_place(t_aspace *as, uint64_t hint, uint64_t len, uint64_t *va);
int64_t		sys_display_info(const t_sysargs *a);
int64_t		sys_display_modes(const t_sysargs *a);
int64_t		sys_display_set_mode(const t_sysargs *a);
int64_t		sys_display_map(const t_sysargs *a);
int64_t		sys_kcon(const t_sysargs *a);
int			selftest_text(void);
int			selftest_modes(void);

extern t_display	g_display;

#endif
