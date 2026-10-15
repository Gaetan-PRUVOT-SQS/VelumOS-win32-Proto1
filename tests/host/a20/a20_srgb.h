#ifndef A20_SRGB_H
# define A20_SRGB_H

# include <stdint.h>

extern const uint32_t	g_srgb_lin_e6[256];

uint32_t	luminance_e6(uint32_t rgb);
uint32_t	ratio_x100(uint32_t fg, uint32_t bg);
uint32_t	blend_over(uint32_t argb, uint32_t rgb);

#endif
