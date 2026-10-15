#include "ctype.h"

int	ispunct(int c)
{
	return (isgraph(c) && !isalnum(c));
}

int	isxdigit(int c)
{
	return (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

int	tolower(int c)
{
	if (isupper(c))
		return (c + ('a' - 'A'));
	return (c);
}

int	toupper(int c)
{
	if (islower(c))
		return (c - ('a' - 'A'));
	return (c);
}
