#ifndef DVM_H
# define DVM_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apkdef.h"
# include "velum/apk/dex.h"
# include "velum/apk/dexcode.h"

# define DVM_THROWN 1
# define DVM_NULL 0u
# define DVM_HEAP_DEFAULT 16777216u
# define DVM_STACK_WORDS_DEFAULT 65536u
# define DVM_DEPTH_DEFAULT 256u
# define DVM_CLASSES_DEFAULT 2048u
# define DVM_LOCALS_MAX 64u

# define DIK_VIRTUAL 0
# define DIK_SUPER 1
# define DIK_DIRECT 2
# define DIK_STATIC 3
# define DIK_INTERFACE 4

# define DAK_INT 0
# define DAK_WIDE 1
# define DAK_OBJECT 2
# define DAK_BOOLEAN 3
# define DAK_BYTE 4
# define DAK_CHAR 5
# define DAK_SHORT 6

typedef uint32_t			t_dref;
typedef struct s_dvm		t_dvm;
typedef struct s_dclass		t_dclass;
typedef struct s_dmethod	t_dmethod;
typedef int					(*t_dnativefn)(t_dvm *vm, const uint32_t *args,
								uint64_t *ret);

typedef struct s_dlimits
{
	uint32_t	heap_bytes;
	uint32_t	stack_words;
	uint32_t	depth_max;
	uint32_t	classes_max;
	uint64_t	budget;
}	t_dlimits;

struct s_dmethod
{
	t_dclass	*cls;
	const char	*name;
	const char	*sig;
	const char	*shorty;
	const t_dex	*dex;
	t_dnativefn	native;
	t_span		insns;
	uint32_t	code_off;
	uint32_t	access;
	uint32_t	slot;
	uint16_t	registers;
	uint16_t	ins;
	uint16_t	outs;
	uint16_t	tries;
};

struct s_dvm
{
	struct s_dheap		*heap;
	struct s_dclasses	*classes;
	void				*host;
	uint32_t			*stack;
	uint32_t			stack_words;
	uint32_t			sp;
	uint32_t			depth;
	t_dref				pending;
	uint64_t			result;
	uint64_t			spent;
	t_dlimits			lim;
	t_dref				locals[DVM_LOCALS_MAX];
	uint32_t			nlocals;
};

typedef struct s_dbuiltin
{
	const char	*desc;
	const char	*super_desc;
	const char	*iface_desc;
	uint32_t	access;
	uint32_t	payload_bytes;
}	t_dbuiltin;

typedef struct s_dnative
{
	const char	*cls;
	const char	*name;
	const char	*sig;
	uint32_t	access;
	t_dnativefn	fn;
}	t_dnative;

typedef struct s_dname
{
	const char	*name;
	const char	*sig;
}	t_dname;

typedef struct s_dstr16
{
	const uint16_t	*p;
	uint32_t		n;
}	t_dstr16;

typedef struct s_darrview
{
	void		*data;
	uint32_t	len;
	uint8_t		width;
	uint8_t		is_ref;
}	t_darrview;

typedef struct s_dheapstats
{
	uint32_t	bytes;
	uint32_t	objects;
	uint32_t	collections;
	uint32_t	peak_bytes;
}	t_dheapstats;

typedef struct s_dfacc
{
	uint32_t	field_idx;
	t_dref		obj;
	uint64_t	val;
	uint8_t		wide;
	uint8_t		is_ref;
	uint8_t		put;
	uint8_t		is_static;
}	t_dfacc;

typedef struct s_daacc
{
	t_dref		arr;
	int32_t		index;
	uint64_t	val;
	uint8_t		kind;
	uint8_t		put;
}	t_daacc;

typedef struct s_dinvoke
{
	uint32_t		method_idx;
	t_dref			self;
	const t_dmethod	*target;
	uint8_t			kind;
}	t_dinvoke;

typedef struct s_dnewarr
{
	uint32_t	type_idx;
	int32_t		len;
	t_dref		out;
}	t_dnewarr;

int			dvm_create(t_dvm **out, const t_dlimits *lim);
void		dvm_destroy(t_dvm *vm);
int			dvm_load_dex(t_dvm *vm, t_span dex);
int			dvm_builtins(t_dvm *vm, const t_dbuiltin *tab, uint32_t n);
int			dvm_natives(t_dvm *vm, const t_dnative *tab, uint32_t n);
int			dvm_class(t_dvm *vm, const char *desc, t_dclass **out);
int			dvm_method(t_dvm *vm, t_dclass *c, const t_dname *name,
				const t_dmethod **out);
int			dvm_new(t_dvm *vm, t_dclass *c, t_dref *out);
t_dclass	*dvm_class_of(t_dvm *vm, t_dref ref);
const char	*dvm_class_name(const t_dclass *c);
int			dvm_is_instance(t_dvm *vm, t_dref ref, const t_dclass *c);
void		*dvm_payload(t_dvm *vm, t_dref ref, const t_dclass *builtin);
int			dvm_string_utf8_new(t_dvm *vm, const char *utf8, t_dref *out);
int			dvm_string_utf16_new(t_dvm *vm, t_dstr16 s, t_dref *out);
int			dvm_string_get(t_dvm *vm, t_dref ref, t_dstr16 *out);
int			dvm_string_utf8(t_dvm *vm, t_dref ref, t_text out);
int			dvm_array_new(t_dvm *vm, const char *desc, int32_t len,
				t_dref *out);
int			dvm_array_view(t_dvm *vm, t_dref ref, t_darrview *out);
int			dvm_throw(t_dvm *vm, const char *desc, const char *msg);
int			dvm_local(t_dvm *vm, t_dref ref);
void		dvm_local_pop(t_dvm *vm, uint32_t n);
int			dvm_pin(t_dvm *vm, t_dref ref);
void		dvm_unpin(t_dvm *vm, t_dref ref);
void		dvm_gc(t_dvm *vm);
void		dvm_heap_stats(const t_dvm *vm, t_dheapstats *out);
int			dvm_call(t_dvm *vm, const t_dmethod *m, const uint32_t *args,
				uint64_t *ret);
int			dvmrt_const_string(t_dvm *vm, const t_dmethod *from, uint32_t idx,
				t_dref *out);
int			dvmrt_const_class(t_dvm *vm, const t_dmethod *from, uint32_t idx,
				t_dref *out);
int			dvmrt_new_instance(t_dvm *vm, const t_dmethod *from, uint32_t idx,
				t_dref *out);
int			dvmrt_new_array(t_dvm *vm, const t_dmethod *from, t_dnewarr *rq);
int			dvmrt_instance_of(t_dvm *vm, const t_dmethod *from, uint32_t idx,
				t_dref obj);
int			dvmrt_catches(t_dvm *vm, const t_dmethod *from, uint32_t idx,
				t_dref exc);
int			dvmrt_field(t_dvm *vm, const t_dmethod *from, t_dfacc *acc);
int			dvmrt_array(t_dvm *vm, t_daacc *acc);
int			dvmrt_array_length(t_dvm *vm, t_dref arr, uint32_t *len);
int			dvmrt_fill_array(t_dvm *vm, t_dref arr, const t_darray *data);
int			dvmrt_resolve(t_dvm *vm, const t_dmethod *from, t_dinvoke *inv);
int			dvm_throwable_message(t_dvm *vm, t_dref exc, t_dref *msg);
int			dvm_throwable_set_message(t_dvm *vm, t_dref exc, t_dref msg);
int			dvm_method_start(const t_dmethod *m, uint32_t pc);

#endif
