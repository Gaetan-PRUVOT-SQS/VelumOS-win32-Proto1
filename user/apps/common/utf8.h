#ifndef UTF8_H
# define UTF8_H

# include <stddef.h>
# include <stdint.h>

size_t	utf8_seq_len(const uint8_t *s, size_t left);
int		utf8_valid(const char *s, size_t len);
size_t	utf8_clean_copy(char *dst, size_t size, const char *src, size_t len);

#endif
