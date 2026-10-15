#ifndef FAKE_UTF8_H
# define FAKE_UTF8_H

# include <stddef.h>
# include <stdint.h>

# define UTF8_ROWS 8
# define UTF8_REPLACEMENT 0xFFFD

typedef struct s_utf8row
{
	uint8_t	lead_lo;
	uint8_t	lead_hi;
	uint8_t	second_lo;
	uint8_t	second_hi;
	uint8_t	len;
}	t_utf8row;

extern const t_utf8row	g_utf8_rows[UTF8_ROWS];

size_t	ref_utf8_encode(uint32_t cp, uint8_t *out);
size_t	ref_utf8_decode(const uint8_t *p, size_t avail, uint32_t *cp);

#endif
