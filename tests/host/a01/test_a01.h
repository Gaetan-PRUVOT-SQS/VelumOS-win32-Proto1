#ifndef TEST_A01_H
# define TEST_A01_H

# include "cpu_int.h"

typedef struct s_trapcase
{
	const char	*name;
	t_trapin	in;
	t_trapact	want;
}	t_trapcase;

#endif
