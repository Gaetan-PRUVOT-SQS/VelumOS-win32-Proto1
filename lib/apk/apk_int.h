#ifndef APK_INT_H
# define APK_INT_H

# include <stdlib.h>
# include "velum/apk/apk.h"
# include "velum/apk/arsc.h"
# include "velum/apk/axml.h"
# include "velum/apk/rsa.h"
# include "velum/crypto.h"
# include "velum/err.h"
# include "velum/libk.h"

# define APK_MAGIC "APK Sig Block 42"
# define APK_MAGIC_LEN 16
# define APK_EOCD_SIG 0x06054b50
# define APK_EOCD_LEN 22
# define APK_ELEM_MAX 32

typedef struct s_sigblock
{
	size_t	off;
	size_t	eocd;
	t_span	v2;
}	t_sigblock;

typedef struct s_signer
{
	t_span	signed_data;
	t_span	digest;
	t_span	cert;
	t_span	sig;
	t_span	pubkey;
	int		found;
	int		reason;
}	t_signer;

typedef struct s_manwalk
{
	t_axml			x;
	const t_apk		*apk;
	t_apkmanifest	*out;
	uint32_t		depth;
	uint32_t		roots;
	uint32_t		apps;
	uint32_t		in_app;
	uint32_t		in_act;
	uint32_t		in_filter;
	uint32_t		has_main;
	uint32_t		has_launcher;
	uint32_t		label_kind;
	uint32_t		label_ref;
	char			name[APK_ELEM_MAX];
	char			cand[APK_CLASS_MAX];
}	t_manwalk;

uint32_t	apk_rd32(const uint8_t *p);
uint64_t	apk_rd64(const uint8_t *p);
int			apk_lp(t_span *in, t_span *out);
int			apk_text_clean(const char *s);
int			sig_locate(const t_apk *a, t_sigblock *b);
void		sig_digest(const t_apk *a, const t_sigblock *b, uint8_t *out);
int			sig_signer(t_span v2, t_signer *s);
int			man_str(const t_axml *x, uint32_t res_id, t_text out);
int			man_u32(const t_axml *x, uint32_t res_id, uint32_t *out);
int			man_package(const t_axml *x, t_text out);
int			man_on_start(t_manwalk *w);
int			man_on_child(t_manwalk *w);
int			man_walk(t_manwalk *w);
int			man_finish(t_manwalk *w);

#endif
