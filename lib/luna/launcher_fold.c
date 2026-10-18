#include "launcher_int.h"

static const char	g_latin1[65] =
	"aaaaaaaceeeeiiiidnoooooxouuuuyts"
	"aaaaaaaceeeeiiiidnoooooxouuuuyty";

static uint32_t	fold_next(const char **p)
{
	const uint8_t	*s;
	uint32_t		c;

	s = (const uint8_t *)*p;
	c = *s;
	if (c == 0xc3 && s[1] >= 0x80 && s[1] <= 0xbf)
	{
		*p += 2;
		return ((uint32_t)(uint8_t)g_latin1[s[1] - 0x80]);
	}
	*p += 1;
	while ((**p & 0xc0) == 0x80 && c >= 0xc0)
		*p += 1;
	if (c >= 'A' && c <= 'Z')
		return (c + ('a' - 'A'));
	return (c);
}

uint32_t	lp_lm_fold(uint32_t *out, uint32_t max, const char *s)
{
	uint32_t	n;

	n = 0;
	while (*s != '\0' && n < max)
		out[n++] = fold_next(&s);
	return (n);
}

static int	same_at(const uint32_t *a, const uint32_t *b, uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		if (a[i] != b[i])
			return (0);
		i++;
	}
	return (1);
}

static int32_t	score_at(const uint32_t *l, uint32_t at)
{
	if (at == 0)
		return (3);
	if (l[at - 1] == ' ' || l[at - 1] == '-' || l[at - 1] == '.'
		|| l[at - 1] == '_')
		return (2);
	return (1);
}

int32_t	luna_lm_match(const char *label, const char *query)
{
	uint32_t	l[LUNA_LM_LABEL_MAX];
	uint32_t	q[LUNA_LM_QUERY_MAX];
	uint32_t	nl;
	uint32_t	nq;
	uint32_t	at;
	int32_t		best;

	if (label == NULL || query == NULL)
		return (0);
	nl = lp_lm_fold(l, LUNA_LM_LABEL_MAX, label);
	nq = lp_lm_fold(q, LUNA_LM_QUERY_MAX, query);
	if (nq == 0)
		return (1);
	best = 0;
	at = 0;
	while (nq <= nl && at <= nl - nq)
	{
		if (same_at(l + at, q, nq) && score_at(l, at) > best)
			best = score_at(l, at);
		at++;
	}
	return (best);
}

int32_t	lp_lm_cmp(const char *a, const char *b)
{
	uint32_t	fa[LUNA_LM_LABEL_MAX];
	uint32_t	fb[LUNA_LM_LABEL_MAX];
	uint32_t	na;
	uint32_t	nb;
	uint32_t	i;

	na = lp_lm_fold(fa, LUNA_LM_LABEL_MAX, a);
	nb = lp_lm_fold(fb, LUNA_LM_LABEL_MAX, b);
	i = 0;
	while (i < na && i < nb)
	{
		if (fa[i] != fb[i])
			return (fa[i] < fb[i] ? -1 : 1);
		i++;
	}
	if (na == nb)
		return (0);
	return (na < nb ? -1 : 1);
}
