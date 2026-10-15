#ifndef CASES_H
# define CASES_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/boot.h"

typedef enum e_geofield
{
	F_NONE = 0,
	F_W,
	F_H,
	F_PITCH,
	F_BPP,
	F_PHYS,
	F_RS,
	F_RZ,
	F_GS,
	F_GZ,
	F_BS,
	F_BZ
}	t_geofield;

typedef struct s_geocase
{
	const char	*name;
	t_geofield	f1;
	uint64_t	v1;
	t_geofield	f2;
	uint64_t	v2;
	int			want;
}	t_geocase;

extern const t_geocase	g_geo_cases[];

void	fake_fb_field(t_fbinfo *fb, t_geofield f, uint64_t v);

typedef struct s_modecase
{
	const char	*name;
	uint32_t	w;
	uint32_t	h;
	bool		want;
}	t_modecase;

typedef struct s_idcase
{
	const char	*name;
	uint32_t	id;
	uint32_t	vram64k;
	uint64_t	bar;
	int			want;
	uint64_t	vram;
}	t_idcase;

typedef struct s_wrapcase
{
	const char	*name;
	const char	*text;
	int32_t		cols;
	int32_t		max;
	int32_t		count;
	bool		trunc;
	const char	*line[4];
}	t_wrapcase;

extern const t_modecase	g_mode_cases[];
extern const t_idcase	g_id_cases[];
extern const t_wrapcase	g_wrap_cases[];

#endif
