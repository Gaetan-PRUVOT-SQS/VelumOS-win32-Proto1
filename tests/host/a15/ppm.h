#ifndef PPM_H
# define PPM_H

# include <stddef.h>
# include <stdint.h>
# include <string.h>
# include "velum/gfx.h"

void		visu_init(const char *argv0);
int			visu_png(const char *name, const t_surface *s, int zoom);
int			ppm_write(const char *path, const t_surface *s, int zoom);
int			png_write(const char *path, const t_surface *s, int zoom);
uint32_t	*visu_alloc(t_surface *s, int32_t w, int32_t h);
int			visu_save(const char *name, t_surface *s, int zoom);
size_t		zlib_stored(uint8_t *out, const uint8_t *raw, size_t len);

#endif
