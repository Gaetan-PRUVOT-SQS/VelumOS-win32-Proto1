#include "ctype.h"

int	isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

int	isupper(int c)
{
	return (c >= 'A' && c <= 'Z');
}

int	islower(int c)
{
	return (c >= 'a' && c <= 'z');
}

int	isalpha(int c)
{
	return (isupper(c) || islower(c));
}

int	isalnum(int c)
{
	return (isalpha(c) || isdigit(c));
}
