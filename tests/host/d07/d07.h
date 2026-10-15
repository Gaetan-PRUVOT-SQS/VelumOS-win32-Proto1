#ifndef D07_H
# define D07_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apk.h"
# include "velum/err.h"

# define D07_LOAD_MAX 0x100000
# define D07_ANY -9000
# define D07_ABSENT -9999
# define D07_MUTATIONS 12000

typedef struct s_d07case
{
	const char	*file;
	int			want;
	int			reason;
}	t_d07case;

t_span		d07_load(const char *name);
void		d07_free(t_span s);
int			d07_verdict(t_span f, int *reason);
int			d07_file_verdict(const char *name, int *reason);
void		d07_table(const t_d07case *c, uint32_t n);
void		d07_fail_after(int n);
uint32_t	d07_rand(uint32_t *s);
void		d07_block_bounds(t_span f, size_t *lo, size_t *hi);
int			d07_try(t_span f);
void		*d07_malloc(size_t n);

#endif
