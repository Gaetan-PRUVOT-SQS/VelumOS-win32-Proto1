#include "ctl_int.h"

void	ctl_copy_text(char *dst, const char *src)
{
	uint32_t	len;
	uint32_t	pos;
	uint32_t	next;

	len = 0;
	if (src)
		len = (uint32_t)strlen(src);
	pos = 0;
	while (pos < len)
	{
		next = ctl_u8_next(src, len, pos);
		if (next > CTL_TEXT_MAX - 1)
			break ;
		pos = next;
	}
	if (pos)
		memmove(dst, src, pos);
	dst[pos] = '\0';
}

static void	strip_amp(const char *text, uint32_t *i, int32_t *mn, int32_t o)
{
	(*i)++;
	if (*mn < 0 && text[*i])
		*mn = o;
}

int32_t	ctl_text_strip(const char *text, char *out, int32_t *mn)
{
	uint32_t	i;
	int32_t		o;

	i = 0;
	o = 0;
	*mn = -1;
	while (text && text[i] && o < CTL_TEXT_MAX - 1)
	{
		if (text[i] == '&' && text[i + 1] == '&')
			i++;
		else if (text[i] == '&')
		{
			strip_amp(text, &i, mn, o);
			continue ;
		}
		out[o] = text[i];
		o++;
		i++;
	}
	out[o] = '\0';
	return (o);
}

int	ctl_mnemonic(const char *text)
{
	char	buf[CTL_TEXT_MAX];
	int32_t	mn;
	int		ch;

	ctl_text_strip(text, buf, &mn);
	if (mn < 0)
		return (0);
	ch = (uint8_t)buf[mn];
	if (ch >= 'a' && ch <= 'z')
		return (ch - 'a' + 'A');
	if ((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
		return (ch);
	return (0);
}
