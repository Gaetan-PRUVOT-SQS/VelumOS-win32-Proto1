#include "dr_int.h"

static const t_dnative	g_dr_lang[] = {
{DR_OBJ, "<init>", "()V", DR_PU, dr_nop},
{DR_OBJ, "toString", "()Ljava/lang/String;", DR_PU, dr_obj_tostring},
{DR_OBJ, "equals", "(Ljava/lang/Object;)Z", DR_PU, dr_obj_equals},
{DR_OBJ, "hashCode", "()I", DR_PU, dr_obj_hash},
{DR_STR, "length", "()I", DR_PU, dr_str_length},
{DR_STR, "charAt", "(I)C", DR_PU, dr_str_charat},
{DR_STR, "equals", "(Ljava/lang/Object;)Z", DR_PU, dr_str_equals},
{DR_STR, "hashCode", "()I", DR_PU, dr_str_hash},
{DR_STR, "toString", "()Ljava/lang/String;", DR_PU, dr_str_self},
{DR_STR, "valueOf", "(I)Ljava/lang/String;", DR_ST, dr_int_tostring},
{DR_STR, "concat", "(Ljava/lang/String;)Ljava/lang/String;", DR_PU,
	dr_str_concat},
{DR_INT, "toString", "(I)Ljava/lang/String;", DR_ST, dr_int_tostring},
{DR_INT, "parseInt", "(Ljava/lang/String;)I", DR_ST, dr_int_parse},
{DR_MATH, "abs", "(I)I", DR_ST, dr_math_abs},
{DR_MATH, "min", "(II)I", DR_ST, dr_math_min},
{DR_MATH, "max", "(II)I", DR_ST, dr_math_max},
{DR_SYS, "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", DR_ST,
	dr_sys_arraycopy},
{DR_SYS, "currentTimeMillis", "()J", DR_ST, dr_sys_millis},
{DR_THR, "<init>", "()V", DR_PU, dr_nop},
{DR_THR, "<init>", "(Ljava/lang/String;)V", DR_PU, dr_thr_init_msg},
{DR_THR, "getMessage", "()Ljava/lang/String;", DR_PU, dr_thr_message},
{DR_SB, "<init>", "()V", DR_PU, dr_nop},
{DR_SB, "append", "(Ljava/lang/String;)Ljava/lang/StringBuilder;", DR_PU,
	dr_sb_append_obj},
{DR_SB, "append", "(Ljava/lang/CharSequence;)Ljava/lang/StringBuilder;",
	DR_PU, dr_sb_append_obj},
{DR_SB, "append", "(Ljava/lang/Object;)Ljava/lang/StringBuilder;", DR_PU,
	dr_sb_append_obj},
{DR_SB, "append", "(I)Ljava/lang/StringBuilder;", DR_PU, dr_sb_append_i},
{DR_SB, "append", "(J)Ljava/lang/StringBuilder;", DR_PU, dr_sb_append_j},
{DR_SB, "append", "(C)Ljava/lang/StringBuilder;", DR_PU, dr_sb_append_c},
{DR_SB, "append", "(Z)Ljava/lang/StringBuilder;", DR_PU, dr_sb_append_z},
{DR_SB, "toString", "()Ljava/lang/String;", DR_PU, dr_sb_tostring},
{DR_SB, "length", "()I", DR_PU, dr_sb_length}
};

uint32_t	dr_tab_lang(const t_dnative **tab)
{
	*tab = g_dr_lang;
	return (sizeof(g_dr_lang) / sizeof(g_dr_lang[0]));
}
