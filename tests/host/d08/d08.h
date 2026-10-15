#ifndef D08_H
# define D08_H

# include <stddef.h>
# include <stdint.h>
# include "velum/err.h"
# include "velum/apk/dvm.h"

typedef struct s_d08fail
{
	int			armed;
	uint32_t	left;
	uint32_t	calls;
}	t_d08fail;

typedef struct s_d08call
{
	uint32_t		calls;
	const t_dmethod	*last;
}	t_d08call;

extern t_d08fail	g_d08;
extern t_d08call	g_d08call;

void	*__real_calloc(size_t n, size_t size);
void	*__wrap_calloc(size_t n, size_t size);
void	d08_fail_after(uint32_t n);
void	d08_fail_off(void);
int		d08_open(t_dvm **vm, t_span *file);
void	d08_close(t_dvm *vm, t_span file);
int		d08_method_idx(t_dvm *vm, const char *cls, const char *name);
int		d08_field_idx(t_dvm *vm, const char *cls, const char *name);

#endif
