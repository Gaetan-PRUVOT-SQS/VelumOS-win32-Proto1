#ifndef AXML_INT_H
# define AXML_INT_H

# include "velum/apk/arsc.h"
# include "velum/err.h"

# define AXML_SKIP 4

typedef struct s_arscpick
{
	uint32_t	res_id;
	const char	*lang;
	int			rank;
	t_resvalue	val;
}	t_arscpick;

uint32_t	res_rd16(const uint8_t *p);
uint32_t	res_rd32(const uint8_t *p);
int			res_chunk(t_span s, size_t off, t_reschunk *c);
int			res_next(t_span s, uint32_t *pos, t_reschunk *c);
uint32_t	res_u8_seq(const uint8_t *s, uint32_t avail);
uint32_t	res_u8_put(uint32_t cp, t_text out, size_t w);
int			respool_u8(const t_respool *p, uint32_t off, t_text out);
int			respool_u16(const t_respool *p, uint32_t off, t_text out);
int			axml_set_map(t_axml *x, const t_reschunk *c);
int			axml_ev_ns(t_axml *x, const t_reschunk *c);
int			axml_ev_start(t_axml *x, const t_reschunk *c);
int			axml_ev_end(t_axml *x, const t_reschunk *c);
int			axml_ev_text(t_axml *x, const t_reschunk *c);
int			arsc_cfg_rank(const uint8_t *cfg, uint32_t n, const char *lang);
int			arsc_entry_value(const uint8_t *t, const t_reschunk *c,
				uint64_t pos, t_resvalue *v);

#endif
