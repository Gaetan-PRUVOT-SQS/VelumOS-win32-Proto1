#ifndef DEXCODE_INT_H
# define DEXCODE_INT_H

# include "velum/apk/dexcode.h"
# include "velum/err.h"

enum e_dflagset
{
	F_END = 0,
	F_N = DO_CONT,
	F_GO = DO_BRANCH,
	F_IF = DO_BRANCH | DO_CONT,
	F_SW = DO_SWITCH | DO_CONT,
	F_FILL = DO_FILL | DO_CONT,
	F_INV = DO_INVOKE | DO_CONT,
	F_NS = DO_NOTSUP | DO_CONT,
	F_RETW = DO_WA,
	W_D = DO_CONT | DO_WDST,
	W_A = DO_CONT | DO_WA,
	W_B = DO_CONT | DO_WB,
	W_DA = DO_CONT | DO_WDST | DO_WA,
	W_DB = DO_CONT | DO_WDST | DO_WB,
	W_BC = DO_CONT | DO_WB | DO_WC,
	W_DBC = DO_CONT | DO_WDST | DO_WB | DO_WC,
	W_DAB = DO_CONT | DO_WDST | DO_WA | DO_WB
};

enum e_dsrc
{
	DS_NO = 0,
	DS_AA,
	DS_A4,
	DS_B4,
	DS_W1,
	DS_W2,
	DS_W3,
	DS_W1L,
	DS_W1H,
	DS_L32
};

enum e_dlit
{
	DL_NO = 0,
	DL_B4,
	DL_AA,
	DL_W1,
	DL_W1H,
	DL_32,
	DL_64
};

enum e_dregs
{
	DR_A = 1,
	DR_B = 2,
	DR_C = 4,
	DR_RANGE = 8,
	DR_LIST = 16
};

typedef struct s_dopinfo
{
	const char	*name;
	uint8_t		fmt;
	uint8_t		kind;
	uint16_t	flags;
}	t_dopinfo;

typedef struct s_dfmtinfo
{
	uint8_t	len;
	uint8_t	a;
	uint8_t	b;
	uint8_t	c;
	uint8_t	lit;
	uint8_t	idx;
	uint8_t	argc;
	uint8_t	regs;
}	t_dfmtinfo;

typedef struct s_dpayload
{
	uint32_t	ident;
	uint32_t	count;
	uint32_t	width;
	uint64_t	units;
}	t_dpayload;

typedef struct s_dscan
{
	t_span					insns;
	const t_dcodelimits		*lim;
	uint8_t					*starts;
	uint32_t				pc;
	uint32_t				n;
	int						live;
}	t_dscan;

extern const t_dopinfo	g_dops[256];

const t_dfmtinfo	*dexcode_fmtinfo(uint8_t fmt);
uint32_t			dexcode_unit(const uint8_t *p, uint64_t i);
uint32_t			dexcode_field(const uint8_t *p, uint8_t src);
int64_t				dexcode_literal(const uint8_t *p, uint8_t src);
int					dexcode_payload(t_span insns, uint32_t pc, t_dpayload *out);
int					dexcode_is_payload(const t_dscan *s, uint32_t pc);
int					dexcode_check(const t_dinsn *in, const t_dcodelimits *lim);
int					dexcode_scan(t_dscan *s);
int					dexcode_targets(t_dscan *s);
void				dexcode_strip(t_dscan *s);
int					dexcode_bit(const uint8_t *starts, uint32_t pc);
uint32_t			dexcode_item_len(const t_dscan *s, uint32_t pc);

#endif
