#include <stdlib.h>
#include <string.h>
#include "a09_test.h"
#include "harness.h"

static int	hex_nibble(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	return (-1);
}

size_t	hex_decode(const char *hex, uint8_t *out, size_t max)
{
	size_t	n;
	int		hi;
	int		lo;

	n = 0;
	if (!strcmp(hex, "-"))
		return (0);
	while (hex[0] && hex[1] && n < max)
	{
		hi = hex_nibble(hex[0]);
		lo = hex_nibble(hex[1]);
		if (hi < 0 || lo < 0)
			return ((size_t)-1);
		out[n++] = (uint8_t)(hi << 4 | lo);
		hex += 2;
	}
	return (n);
}

void	hex_encode(const uint8_t *in, size_t n, char *out)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		out[i * 2] = "0123456789abcdef"[in[i] >> 4];
		out[i * 2 + 1] = "0123456789abcdef"[in[i] & 15];
		i++;
	}
	out[n * 2] = '\0';
}

void	h_eq_hex(const char *what, const uint8_t *got, size_t n,
		const char *want_hex)
{
	char	*hex;

	hex = malloc(n * 2 + 1);
	h_true(hex != NULL, "allocation du texte hexadecimal");
	if (!hex)
		return ;
	hex_encode(got, n, hex);
	h_eq_str(what, hex, want_hex);
	free(hex);
}

void	fill_pattern(uint8_t *buf, size_t n, uint32_t seed)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		seed = seed * 1664525u + 1013904223u;
		buf[i] = (uint8_t)(seed >> 24);
		i++;
	}
}
