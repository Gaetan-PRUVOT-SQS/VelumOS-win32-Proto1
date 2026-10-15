#ifndef LT_HIT_H
# define LT_HIT_H

# include <stdint.h>
# include "lt_geo.h"

# define LT_BAND 34
# define LT_BAND_X 40
# define LT_BAND_BTN 110

typedef struct s_lthit
{
	const char	*name;
	t_rect		outer;
	uint32_t	style;
	bool		maximized;
	t_point		p;
	uint32_t	want;
}	t_lthit;

typedef struct s_ltsweep
{
	uint32_t	count[21];
	uint32_t	edge_client;
	uint32_t	edge_close;
	uint32_t	edge_sys;
	uint32_t	bad;
	int32_t		bad_x;
	int32_t		bad_y;
}	t_ltsweep;

extern const t_lthit	g_hit_cases[];
extern const int		g_hit_count;

bool		lt_ht_valid(uint32_t h);
bool		lt_ht_edge(uint32_t h);
uint32_t	lt_ht_mirror(uint32_t h);
bool		lt_sweep_visit(const t_lunawin *w, t_point p, int32_t step);
void		lt_sweep(const t_lunawin *w, int32_t step, t_ltsweep *out);
uint32_t	lt_area(t_rect r);
void		lt_sweep_report(const t_lunawin *w, const t_ltsweep *s);
void		lt_sweep_counts(const t_lunawin *w, const t_ltsweep *s);
void		lt_same_zones(const t_lunawin *a, const t_lunawin *b,
				const char *msg);
uint32_t	lt_rep_style(uint32_t i);

#endif
