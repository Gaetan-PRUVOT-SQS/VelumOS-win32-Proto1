#include "ctype.h"

int	isspace(int c)
{
	return (c == ' ' || (c >= '\t' && c <= '\r'));
}

int	isblank(int c)
{
	return (c == ' ' || c == '\t');
}

int	iscntrl(int c)
{
	return ((c >= 0 && c < ' ') || c == 127);
}

int	isprint(int c)
{
	return (c >= ' ' && c <= '~');
}

int	isgraph(int c)
{
	return (c > ' ' && c <= '~');
}
