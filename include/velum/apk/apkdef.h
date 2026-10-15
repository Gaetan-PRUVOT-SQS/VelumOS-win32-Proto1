#ifndef APKDEF_H
# define APKDEF_H

# include <stddef.h>
# include <stdint.h>

typedef struct s_span
{
	const uint8_t	*p;
	size_t			len;
}	t_span;

typedef struct s_text
{
	char	*p;
	size_t	cap;
}	t_text;

#endif
