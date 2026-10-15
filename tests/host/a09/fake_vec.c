#include <stdio.h>
#include <string.h>
#include "a09_test.h"

static int	vec_split(char *line, t_vecline *out)
{
	char	*tok;
	char	*save;

	out->n = 0;
	tok = strtok_r(line, " \t\r\n", &save);
	while (tok && out->n < VEC_MAX_FIELDS)
	{
		out->f[out->n++] = tok;
		tok = strtok_r(NULL, " \t\r\n", &save);
	}
	return (out->n);
}

int	vec_foreach(const char *path, void (*cb)(const t_vecline *))
{
	char		line[VEC_LINE_MAX];
	t_vecline	vl;
	FILE		*fp;
	int			count;
	int			lineno;

	fp = fopen(path, "r");
	if (!fp)
		return (-1);
	count = 0;
	lineno = 0;
	while (fgets(line, sizeof(line), fp))
	{
		lineno++;
		vl.lineno = lineno;
		if (line[0] != '#' && vec_split(line, &vl) > 0)
		{
			cb(&vl);
			count++;
		}
	}
	fclose(fp);
	return (count);
}
