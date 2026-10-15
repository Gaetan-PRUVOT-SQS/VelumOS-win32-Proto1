#ifndef D09_H
# define D09_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/dvm.h"
# include "velum/err.h"

# define FK_OBJS 64
# define FK_METHODS 128
# define FK_FIELDS 64
# define FK_SLOTS 8
# define FK_STACK 1024
# define FK_NPE "Ljava/lang/NullPointerException;"

typedef struct s_fkobj
{
	const char	*desc;
	uint64_t	*data;
	uint32_t	len;
	uint8_t		is_ref;
}	t_fkobj;

typedef struct s_fk
{
	t_dvm		vm;
	t_dex		dex;
	uint8_t		*file;
	t_fkobj		obj[FK_OBJS];
	uint32_t	nobj;
	t_dmethod	meth[FK_METHODS];
	uint8_t		known[FK_METHODS];
	uint64_t	statics[FK_FIELDS];
	t_dnativefn	native;
	uint16_t	native_ins;
	uint32_t	args[4];
	uint8_t		code[64];
}	t_fk;

typedef struct s_case
{
	const char	*what;
	uint8_t		op;
	uint64_t	x;
	uint64_t	y;
	uint64_t	want;
	int			rc;
}	t_case;

extern t_fk		g_fk;

void			fk_init(const char *dex, uint32_t depth, uint64_t budget);
void			fk_end(void);
void			fk_check(const char *what);
t_dref			fk_obj(const char *desc, uint32_t slots);
const t_dmethod	*fk_method(const char *cls, const char *name);
int				fk_calc(const char *cls, const char *name, uint64_t *ret);
int				fk_units(const uint16_t *u, uint32_t n, const t_case *c,
					uint64_t *ret);
void			fk_play(const uint16_t *u, uint32_t n, const t_case *c);
void			fk_case3(const t_case *c);
void			fk_case2(const t_case *c);
void			fk_case2a(const t_case *c);
void			fk_caselit(const t_case *c);
void			fk_table(const t_case *t, void (*fn)(const t_case *));
int				fk_same(const char *a, const char *b);
const char		*fk_pending(void);
const char		*fk_type(uint32_t idx);
uint64_t		fk_dbits(double d);
uint32_t		fk_fbits(float f);

#endif
