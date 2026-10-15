#ifndef D00_H
# define D00_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apkdef.h"
# include "velum/apk/arsc.h"
# include "velum/apk/axml.h"
# include "velum/apk/dex.h"
# include "velum/apk/dexcode.h"
# include "velum/apk/zip.h"
# include "velum/err.h"

# define D00_TEXT 256

typedef struct s_d00apk
{
	t_span	file;
	t_zip	zip;
	t_span	manifest;
	t_span	arsc;
	t_span	dex;
}	t_d00apk;

t_span		d00_load(const char *name);
void		d00_free(t_span s);
t_span		d00_entry(const t_zip *z, const char *name);
int			d00_apk_open(t_d00apk *a, const char *name);
void		d00_apk_close(t_d00apk *a);
uint32_t	d00_verify_all(const t_dex *d, uint32_t *refused);
int			d00_attr_named(const t_axml *x, const char *name, t_axmlattr *out);
int			d00_same(const char *a, const char *b);

#endif
