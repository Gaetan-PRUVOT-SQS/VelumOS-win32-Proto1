#include <stdlib.h>
#include <string.h>
#include "a16_test.h"

uint64_t	rng_next(uint64_t *state)
{
	*state ^= *state >> 12;
	*state ^= *state << 25;
	*state ^= *state >> 27;
	return (*state * 0x2545F4914F6CDD1Dull);
}

uint64_t	rng_seed(void)
{
	const char	*env;

	env = getenv("A16_SEED");
	if (env && *env)
		return (strtoull(env, NULL, 0));
	return (0x16A16ull);
}

size_t	model_boundary(const char *s, size_t len, size_t from)
{
	uint32_t	cp;

	if (from >= len || s[from] == '\0')
		return (from);
	return (from + ref_utf8_decode((const uint8_t *)s + from, len - from, &cp));
}

int64_t	model_width(const t_font *f, const char *s, size_t len)
{
	size_t			at;
	size_t			next;
	uint32_t		cp;
	int64_t			sum;

	at = 0;
	sum = 0;
	while (at < len && s[at] != '\0')
	{
		next = at + ref_utf8_decode((const uint8_t *)s + at, len - at, &cp);
		sum += font_glyph(f, cp)->advance;
		at = next;
	}
	return (sum);
}

int	model_is_boundary(const char *s, size_t len, size_t at)
{
	size_t	cur;

	cur = 0;
	while (cur < at)
		cur = model_boundary(s, len, cur);
	return (cur == at);
}
