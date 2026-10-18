#include <stdint.h>

void luna_theme_set(int theme) __attribute__((weak));
void luna_transparency_set(int reduced) __attribute__((weak));

void wm_luna_apply(uint32_t dark, uint32_t reduced)
{
	if (luna_theme_set)
		luna_theme_set(dark ? 1 : 0);
	if (luna_transparency_set)
		luna_transparency_set(reduced != 0);
}
