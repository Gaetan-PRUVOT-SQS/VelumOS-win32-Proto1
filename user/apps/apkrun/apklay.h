#ifndef APKLAY_H
# define APKLAY_H

# include <stddef.h>
# include <stdint.h>
# include "velum/gfx.h"
# include "velum/apk/droid.h"

# define APKLAY_MARGIN 12
# define APKLAY_TEXT_H 20
# define APKLAY_BUTTON_H 26
# define APKLAY_MIN_W 8
# define APKLAY_ZONE_MAX 16384

typedef struct s_apkbox
{
	uint32_t	view;
	uint32_t	kind;
	t_rect		r;
}	t_apkbox;

typedef struct s_apklay
{
	t_apkbox	box[DROID_VIEWS_MAX];
	uint32_t	n;
	uint64_t	seen;
}	t_apklay;

void	apklay_run(const t_droid *d, t_rect zone, t_apklay *out);

#endif
