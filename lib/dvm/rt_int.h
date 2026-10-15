#ifndef RT_INT_H
# define RT_INT_H

# include <stdlib.h>
# include <string.h>
# include "velum/err.h"
# include "velum/apk/dvm.h"

# define DOK_FREE 0
# define DOK_OBJECT 1
# define DOK_STRING 2
# define DOK_ARRAY 3
# define DOK_ANY 255
# define DOBJ_COST 16u
# define DOBJ_FIRST 64u
# define DOBJ_MAX 0x08000000u
# define DSTR_MAX 0x0fffffffu
# define DDESC_MAX 255u
# define DPAYLOAD_MAX 1048576u
# define DMSG_MAX 240u
# define DJOIN_MAX 512
# define DCLASSES_HARD 65536u
# define DSTACK_HARD 16777216u
# define DNAT_TABS 8u
# define DLINK_DEPTH 64u
# define DIFACES_MAX 64u
# define DMEMBERS_MAX 4096u
# define DWORDS_MAX 65535u
# define DX_NPE "Ljava/lang/NullPointerException;"
# define DX_CCE "Ljava/lang/ClassCastException;"
# define DX_AIOOBE "Ljava/lang/ArrayIndexOutOfBoundsException;"
# define DX_ASE "Ljava/lang/ArrayStoreException;"
# define DX_VERIFY "Ljava/lang/VerifyError;"
# define DX_NOCLASS "Ljava/lang/NoClassDefFoundError;"
# define DCORE_OBJECT 0
# define DCORE_CLASS 1
# define DCORE_CHARSEQ 2
# define DCORE_STRING 3
# define DCORE_THROWABLE 4
# define DCORE_N 5

typedef struct s_dobj
{
	t_dclass	*cls;
	void		*data;
	uint32_t	len;
	uint32_t	bytes;
	uint32_t	next;
	uint16_t	pins;
	uint8_t		kind;
	uint8_t		mark;
}	t_dobj;

typedef struct s_dnewobj
{
	t_dclass	*cls;
	uint64_t	bytes;
	uint32_t	len;
	uint8_t		kind;
}	t_dnewobj;

struct s_dheap
{
	t_dobj			*tab;
	uint32_t		cap;
	uint32_t		free_head;
	uint32_t		limit;
	t_dref			oom;
	t_dheapstats	st;
};

typedef struct s_dfield
{
	const char	*name;
	const char	*type;
	uint32_t	slot;
	uint8_t		is_static;
	uint8_t		wide;
	uint8_t		is_ref;
}	t_dfield;

struct s_dclass
{
	char		*desc;
	t_dclass	*super;
	t_dclass	*iface;
	t_dclass	*elem;
	t_dclass	**ifaces;
	t_dmethod	*methods;
	uint8_t		**starts;
	char		**sigs;
	t_dfield	*fields;
	uint32_t	*statics;
	uint32_t	nifaces;
	uint32_t	nmethods;
	uint32_t	nfields;
	uint32_t	nstatics;
	t_dref		mirror;
	uint32_t	init;
	uint32_t	access;
	uint32_t	words;
	uint32_t	pay_off;
	uint32_t	pay_end;
	uint8_t		width;
	uint8_t		is_ref;
	uint8_t		is_array;
};

struct s_dclasses
{
	t_dclass	**tab;
	uint32_t	n;
	uint32_t	max;
	t_dclass	*core[DCORE_N];
	t_dex		dex;
	uint8_t		*def_state;
	t_dref		*interned;
	t_dmethod	*natm[DNAT_TABS];
	uint32_t	natn[DNAT_TABS];
	uint32_t	nnat;
	uint32_t	has_dex;
	uint32_t	depth;
};

t_dobj		*rt_obj(const t_dvm *vm, t_dref ref, uint8_t kind);
uint32_t	*rt_words(const t_dobj *o);
int			rt_grow(struct s_dheap *h);
void		rt_oom_retype(t_dvm *vm);
void		rt_release(struct s_dheap *h, uint32_t i);
int			rt_alloc(t_dvm *vm, const t_dnewobj *rq, t_dref *out);
t_dclass	*rt_find(const t_dvm *vm, const char *desc);
int			rt_define(t_dvm *vm, const t_dbuiltin *b, t_dclass **out);
int			rt_assignable(const t_dclass *from, const t_dclass *to);
int			rt_array_class(t_dvm *vm, const char *desc, t_dclass **out);
int			rt_exc_new(t_dvm *vm, t_dclass *c, const char *msg, t_dref *out);
int			rt_utf8_to16(const char *utf8, uint16_t *out);
int			rt_utf16_to8(t_dstr16 s, t_text out);
void		rt_mark(const t_dvm *vm, uint32_t value);
void		rt_roots_classes(const t_dvm *vm);
char		*rt_dup(const char *s);
void		rt_class_free(t_dclass *c);
const char	*rt_dstr(const t_dex *d, uint32_t idx);
const char	*rt_dtype(const t_dex *d, uint32_t idx);
uint16_t	rt_sig_words(const char *s);
char		*rt_sig_build(const t_dex *d, const t_dexproto *p);
int			rt_verr(t_dvm *vm, const t_dclass *c);
t_dmethod	*rt_method_own(const t_dvm *vm, const t_dclass *c,
				const t_dname *nm);
t_dmethod	*rt_method_find(const t_dvm *vm, const t_dclass *c,
				const t_dname *nm);
int			rt_clinit(t_dvm *vm, t_dclass *c);
int			rt_link(t_dvm *vm, const char *desc, t_dclass **out);
int			rt_members(t_dvm *vm, const t_dexclass *dc, t_dclass *c);
int			rt_method_fill(t_dvm *vm, const t_dexmember *mb, t_dclass *c,
				uint32_t i);
int			rt_verify(t_dvm *vm, t_dclass *c);
int			rt_intern(t_dvm *vm, uint32_t idx, t_dref *out);
int			rt_statics_init(t_dvm *vm, const t_dexclass *dc, t_dclass *c);
size_t		rt_cat(char *buf, size_t n, const char *s);
int			rt_class_idx(t_dvm *vm, uint32_t type_idx, t_dclass **out);
void		rt_aput(const t_dobj *o, const t_daacc *a);

#endif
