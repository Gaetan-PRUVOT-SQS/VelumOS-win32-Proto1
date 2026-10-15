#ifndef D11_H
# define D11_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/droid.h"
# include "velum/err.h"

# define D11_TXT 2048
# define D11_THROWN -99999
# define D11_MIN 0x80000000u
# define D11_MAX 0x7fffffffu
# define X_NPE "Ljava/lang/NullPointerException;"
# define X_CCE "Ljava/lang/ClassCastException;"
# define X_ISE "Ljava/lang/IllegalStateException;"
# define X_IAE "Ljava/lang/IllegalArgumentException;"
# define X_NFE "Ljava/lang/NumberFormatException;"
# define X_ASE "Ljava/lang/ArrayStoreException;"
# define X_OOME "Ljava/lang/OutOfMemoryError;"
# define X_SIOOBE "Ljava/lang/StringIndexOutOfBoundsException;"
# define X_AIOOBE "Ljava/lang/ArrayIndexOutOfBoundsException;"
# define M_ABS "Ljava/lang/Math;|abs|(I)I"
# define M_MIN "Ljava/lang/Math;|min|(II)I"
# define M_MAX "Ljava/lang/Math;|max|(II)I"
# define M_PARSE "Ljava/lang/Integer;|parseInt|(Ljava/lang/String;)I"
# define M_ITOS "Ljava/lang/Integer;|toString|(I)Ljava/lang/String;"
# define M_VALUEOF "Ljava/lang/String;|valueOf|(I)Ljava/lang/String;"
# define M_LEN "Ljava/lang/String;|length|()I"
# define M_CHARAT "Ljava/lang/String;|charAt|(I)C"
# define M_SEQ "Ljava/lang/String;|equals|(Ljava/lang/Object;)Z"
# define M_SHASH "Ljava/lang/String;|hashCode|()I"
# define M_CONCAT "$S|concat|($S)$S"
# define M_OSTR "Ljava/lang/Object;|toString|()Ljava/lang/String;"
# define M_OEQ "Ljava/lang/Object;|equals|(Ljava/lang/Object;)Z"
# define M_OHASH "Ljava/lang/Object;|hashCode|()I"
# define SB "Ljava/lang/StringBuilder;"
# define M_SBS "$B|append|($S)$B"
# define M_SBO "$B|append|($O)$B"
# define M_SBI "$B|append|(I)$B"
# define M_SBJ "$B|append|(J)$B"
# define M_SBC "$B|append|(C)$B"
# define M_SBZ "$B|append|(Z)$B"
# define M_SBSTR "Ljava/lang/StringBuilder;|toString|()Ljava/lang/String;"
# define M_SBLEN "Ljava/lang/StringBuilder;|length|()I"
# define M_ACOPY "Ljava/lang/System;|arraycopy|($OI$OII)V"
# define M_MILLIS "Ljava/lang/System;|currentTimeMillis|()J"
# define M_GETMSG "Ljava/lang/Throwable;|getMessage|()Ljava/lang/String;"
# define TV "Landroid/widget/TextView;"
# define LL "Landroid/widget/LinearLayout;"
# define M_SETTEXT "$T|setText|($C)V"
# define M_GETTEXT "$T|getText|()$C"
# define M_ADD "Landroid/view/ViewGroup;|addView|(Landroid/view/View;)V"
# define M_ORIENT "Landroid/widget/LinearLayout;|setOrientation|(I)V"
# define M_LISTEN "Landroid/view/View;|setOnClickListener|($L)V"
# define M_LOGD "Landroid/util/Log;|d|(Ljava/lang/String;Ljava/lang/String;)I"
# define M_LOGI "Landroid/util/Log;|i|(Ljava/lang/String;Ljava/lang/String;)I"
# define M_LOGW "Landroid/util/Log;|w|(Ljava/lang/String;Ljava/lang/String;)I"
# define M_TVINIT "$T|<init>|(Landroid/content/Context;)V"
# define M_LOGE "Landroid/util/Log;|e|(Ljava/lang/String;Ljava/lang/String;)I"

typedef struct s_d11
{
	t_span		file;
	t_dvm		*vm;
	t_droid		d;
	uint32_t	nlogs;
	uint32_t	level;
	uint64_t	now;
	uint64_t	ret;
	char		log[512];
	char		exc[256];
	char		cls[128];
	char		name[64];
	char		sig[128];
	char		txt[D11_TXT];
}	t_d11;

int			d11_open(t_d11 *t, const char *dex, uint32_t heap);
void		d11_close(t_d11 *t);
int			d11_has(const char *hay, const char *needle);
int			d11_nat(t_d11 *t, const char *ref, const uint32_t *args);
int64_t		d11_i2(t_d11 *t, const char *ref, uint32_t a, uint32_t b);
int64_t		d11_calcul(t_d11 *t, const char *desc);
t_dref		d11_str(t_d11 *t, const char *s);
const char	*d11_text(t_d11 *t, t_dref ref);
t_dref		d11_new(t_d11 *t, const char *desc);
t_dref		d11_view(t_d11 *t, const char *desc);
void		d11_expand(char *dst, size_t cap, const char *src);
int64_t		d11_parse(t_d11 *t, const char *s);
int64_t		d11_copie(t_d11 *t, const uint32_t *a);
int32_t		*d11_entiers(t_d11 *t, t_dref *out, int32_t len);
const char	*d11_long(char *buf, size_t n, const char *fin);

#endif
