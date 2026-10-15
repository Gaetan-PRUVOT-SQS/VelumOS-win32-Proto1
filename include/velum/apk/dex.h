#ifndef DEX_H
# define DEX_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apkdef.h"

# define DEX_NO_INDEX 0xffffffffu
# define DEX_T_STRING 0
# define DEX_T_TYPE 1
# define DEX_T_PROTO 2
# define DEX_T_FIELD 3
# define DEX_T_METHOD 4
# define DEX_T_CLASS 5
# define DEX_M_SFIELD 0
# define DEX_M_IFIELD 1
# define DEX_M_DIRECT 2
# define DEX_M_VIRTUAL 3
# define DEX_V_BYTE 0x00
# define DEX_V_SHORT 0x02
# define DEX_V_CHAR 0x03
# define DEX_V_INT 0x04
# define DEX_V_LONG 0x06
# define DEX_V_FLOAT 0x10
# define DEX_V_DOUBLE 0x11
# define DEX_V_STRING 0x17
# define DEX_V_NULL 0x1e
# define DEX_V_BOOLEAN 0x1f
# define ACC_PUBLIC 0x1
# define ACC_PRIVATE 0x2
# define ACC_PROTECTED 0x4
# define ACC_STATIC 0x8
# define ACC_FINAL 0x10
# define ACC_SYNCHRONIZED 0x20
# define ACC_VOLATILE 0x40
# define ACC_BRIDGE 0x40
# define ACC_TRANSIENT 0x80
# define ACC_VARARGS 0x80
# define ACC_NATIVE 0x100
# define ACC_INTERFACE 0x200
# define ACC_ABSTRACT 0x400
# define ACC_STRICT 0x800
# define ACC_SYNTHETIC 0x1000
# define ACC_ANNOTATION 0x2000
# define ACC_ENUM 0x4000
# define ACC_CONSTRUCTOR 0x10000
# define ACC_DECLARED_SYNCHRONIZED 0x20000

typedef struct s_dex
{
	const uint8_t	*p;
	size_t			len;
	uint32_t		n[6];
	uint32_t		off[6];
}	t_dex;

typedef struct s_dexstr
{
	const char	*p;
	uint32_t	size;
	uint32_t	utf16;
}	t_dexstr;

typedef struct s_dexlist
{
	const uint8_t	*p;
	uint32_t		n;
}	t_dexlist;

typedef struct s_dexproto
{
	uint32_t	shorty_idx;
	uint32_t	return_idx;
	t_dexlist	params;
}	t_dexproto;

typedef struct s_dexfield
{
	uint32_t	class_idx;
	uint32_t	type_idx;
	uint32_t	name_idx;
}	t_dexfield;

typedef struct s_dexmethod
{
	uint32_t	class_idx;
	uint32_t	proto_idx;
	uint32_t	name_idx;
}	t_dexmethod;

typedef struct s_dexclass
{
	uint32_t	class_idx;
	uint32_t	access;
	uint32_t	superclass_idx;
	t_dexlist	interfaces;
	uint32_t	source_file_idx;
	uint32_t	class_data_off;
	uint32_t	static_values_off;
}	t_dexclass;

typedef struct s_dexcdata
{
	const t_dex	*d;
	uint32_t	pos;
	uint32_t	left[4];
	uint32_t	kind;
	uint32_t	last;
}	t_dexcdata;

typedef struct s_dexmember
{
	uint32_t	kind;
	uint32_t	idx;
	uint32_t	access;
	uint32_t	code_off;
}	t_dexmember;

typedef struct s_dexcode
{
	const uint8_t	*insns;
	uint32_t		registers;
	uint32_t		ins;
	uint32_t		outs;
	uint32_t		tries;
	uint32_t		insns_size;
	uint32_t		off;
	uint32_t		tries_off;
	uint32_t		handlers_off;
}	t_dexcode;

typedef struct s_dexhit
{
	const t_dex	*d;
	uint32_t	pos;
	uint32_t	left;
	uint32_t	catch_all;
	uint32_t	insns_size;
}	t_dexhit;

typedef struct s_dexcatch
{
	uint32_t	type_idx;
	uint32_t	addr;
}	t_dexcatch;

typedef struct s_dexvalue
{
	uint32_t	kind;
	int64_t		i;
}	t_dexvalue;

int			dex_open(t_dex *d, t_span file);
int			dex_string(const t_dex *d, uint32_t idx, t_dexstr *out);
int			dex_type(const t_dex *d, uint32_t type_idx, t_dexstr *out);
int			dex_proto(const t_dex *d, uint32_t proto_idx, t_dexproto *out);
int			dex_field(const t_dex *d, uint32_t field_idx, t_dexfield *out);
int			dex_method(const t_dex *d, uint32_t method_idx, t_dexmethod *out);
int			dex_class(const t_dex *d, uint32_t def_idx, t_dexclass *out);
int			dex_find_class(const t_dex *d, const char *descriptor);
int			dex_cdata_open(const t_dex *d, const t_dexclass *c, t_dexcdata *it);
int			dex_cdata_next(t_dexcdata *it, t_dexmember *out);
int			dex_code(const t_dex *d, uint32_t code_off, t_dexcode *out);
int			dex_try_find(const t_dex *d, const t_dexcode *code, uint32_t pc);
int			dex_handler_open(const t_dex *d, const t_dexcode *code,
				uint32_t try_index, t_dexhit *it);
int			dex_handler_next(t_dexhit *it, t_dexcatch *out);
int			dex_static_value(const t_dex *d, const t_dexclass *c,
				uint32_t ordinal, t_dexvalue *out);
int			dex_type_list(const t_dex *d, uint32_t off, t_dexlist *out);
int			mutf8_to_utf8(const char *in, t_text out);
uint32_t	dex_adler32(const uint8_t *p, size_t n);
uint32_t	dex_list_at(const t_dexlist *l, uint32_t i);

#endif
