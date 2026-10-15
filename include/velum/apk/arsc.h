#ifndef ARSC_H
# define ARSC_H

# include "velum/apk/axml.h"

# define ARSC_MAX_REF 8
# define RES_NO_ENTRY 0xffffffff
# define RES_ENTRY_COMPLEX 0x0001
# define RES_ENTRY_COMPACT 0x0008

typedef struct s_resvalue
{
	uint32_t	type;
	uint32_t	data;
}	t_resvalue;

typedef struct s_resquery
{
	uint32_t	res_id;
	const char	*lang;
}	t_resquery;

typedef struct s_arsc
{
	t_span		file;
	t_respool	strings;
	uint32_t	body;
	uint32_t	has_pool;
	uint32_t	packages;
}	t_arsc;

int	arsc_open(t_arsc *a, t_span file);
int	arsc_lookup(const t_arsc *a, uint32_t res_id, const char *lang,
		t_resvalue *out);
int	arsc_string(const t_arsc *a, const t_resquery *q, t_text out);

#endif
