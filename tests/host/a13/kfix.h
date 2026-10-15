#ifndef KFIX_H
# define KFIX_H

# include "fakes.h"
# include "kcon_int.h"

typedef struct s_kfix
{
	t_gsurf	g;
	t_kcon	k;
}	t_kfix;

bool			kfix_open(t_kfix *f, int32_t w, int32_t h, int32_t stride);
void			kfix_close(t_kfix *f);
void			kfix_feed(t_kfix *f, const char *s);
const t_fdraw	*kfix_glyph(uint32_t back);
uint32_t		kfix_cell_fg(const t_kfix *f, int32_t col, int32_t row);
bool			kfix_is(const t_kfix *f, uint32_t i, uint32_t cp, int32_t at);
bool			kfix_same(const t_kfix *a, const t_kfix *b);
void			kfix_feed_n(t_kfix *f, const char *s, size_t n);
void			kfix_snap(const t_kfix *f, uint32_t *copy);
uint32_t		kfix_diff(const t_kfix *f, const uint32_t *snap);
uint32_t		kfix_spaces(void);
int				kfix_try(const t_surface *s);
bool			kfix_ok(const t_kfix *f);
uint64_t		kfix_rnd(uint64_t *s);
void			kfix_stream(char *buf, size_t n);
void			fake_text_gen(char *buf, size_t size, uint64_t *seed);
int				fake_wrapck(const t_bsod_wrap *w, const char *t, int32_t c);
int				fake_wrapu8(const t_bsod_wrap *w);
bool			fake_log_has(const char *ascii);
bool			fx_display(uint32_t w, uint32_t h);
bool			fx_rc_ok(int64_t rc);
int				fx_fuzz_one(uint64_t *seed, uint8_t *buf);
uint32_t		fx_canary(const uint8_t *p, size_t n);
uint64_t		fx_u(const void *p);
int64_t			fx_map(uint64_t hint);
const t_fmap	*fx_live_map(int n);
int				kfix_chunked(t_kfix *f, const char *buf, size_t n);
int				kfix_fuzz_once(t_kfix *f, uint64_t *seed);

#endif
