#include "velum/err.h"
#include "console.h"

static int	is_blank(char c)
{
	return (c == ' ' || c == '\t');
}

static char	*skip_blank(char *p)
{
	while (is_blank(*p))
		p++;
	return (p);
}

static int	take_word(char **pp, char **out)
{
	char	*r;
	char	*w;
	char	q;

	r = *pp;
	w = r;
	*out = w;
	q = 0;
	while (*r && (q || !is_blank(*r)))
	{
		if (!q && (*r == '"' || *r == '\''))
			q = *r;
		else if (q && *r == q)
			q = 0;
		else
			*w++ = *r;
		r++;
	}
	if (q)
		return (E_INVAL);
	if (*r)
		r++;
	*w = '\0';
	*pp = r;
	return (0);
}

int	con_split(char *line, char **argv, int max)
{
	int	n;

	n = 0;
	line = skip_blank(line);
	while (*line)
	{
		if (n >= max)
			return (E_RANGE);
		if (take_word(&line, &argv[n]) < 0)
			return (E_INVAL);
		n++;
		line = skip_blank(line);
	}
	return (n);
}
