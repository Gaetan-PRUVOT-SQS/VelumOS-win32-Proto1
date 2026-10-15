#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "velum/font.h"

#define RECORD_MAX 16

static unsigned char	*slurp(size_t *len)
{
	unsigned char	*buf;
	unsigned char	*bigger;
	size_t			cap;

	cap = 1 << 20;
	*len = 0;
	buf = malloc(cap);
	while (buf)
	{
		*len += fread(buf + *len, 1, cap - *len, stdin);
		if (*len < cap)
			return (buf);
		cap *= 2;
		bigger = realloc(buf, cap);
		if (!bigger)
			free(buf);
		buf = bigger;
	}
	return (NULL);
}

static size_t	decode_record(const unsigned char *rec, size_t n,
		unsigned char *out)
{
	unsigned char	buf[RECORD_MAX];
	const char		*cur;
	size_t			count;
	uint32_t		cp;

	memset(buf, 0x80, sizeof(buf));
	memcpy(buf, rec, n);
	cur = (const char *)buf;
	count = 0;
	while (cur < (const char *)buf + n)
	{
		cp = font_utf8_next(&cur, (const char *)buf + n);
		memcpy(out + 1 + count * 4, &cp, 4);
		count++;
	}
	out[0] = (unsigned char)count;
	return (1 + count * 4);
}

static int	run_records(const unsigned char *in, size_t len, unsigned char *out)
{
	size_t	at;
	size_t	used;

	at = 0;
	used = 0;
	while (at < len)
	{
		if (in[at] == 0 || in[at] > 8 || at + 1 + in[at] > len)
			return (2);
		used += decode_record(in + at + 1, in[at], out + used);
		at += 1 + in[at];
	}
	fwrite(out, 1, used, stdout);
	return (0);
}

int	main(void)
{
	unsigned char	*in;
	unsigned char	*out;
	size_t			len;
	int				rc;

	in = slurp(&len);
	out = NULL;
	if (in)
		out = malloc(len * 5 + 1);
	rc = 1;
	if (in && out)
		rc = run_records(in, len, out);
	free(in);
	free(out);
	return (rc);
}
