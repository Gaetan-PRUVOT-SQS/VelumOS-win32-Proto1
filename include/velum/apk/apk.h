#ifndef APK_H
# define APK_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apkdef.h"
# include "velum/apk/zip.h"

# define APK_FILE_MAX 0x4000000
# define APK_ENTRY_MAX 0x1000000
# define APK_CHUNK 0x100000
# define APK_PKG_MAX 128
# define APK_LABEL_MAX 64
# define APK_CLASS_MAX 192
# define APK_DESC_MAX 196
# define APK_VNAME_MAX 32
# define APK_PERM_MAX 16
# define APK_PERM_LEN 64
# define APK_SIG_V2_ID 0x7109871a
# define APK_ALGO_RSA_PKCS1_SHA256 0x0103

typedef enum e_apkr
{
	APKR_OK,
	APKR_ARCHIVE,
	APKR_TAILLE,
	APKR_NOM,
	APKR_NATIF,
	APKR_MULTIDEX,
	APKR_MANQUE,
	APKR_SIGNATURE,
	APKR_ALGO,
	APKR_SIGNATAIRES,
	APKR_MANIFESTE,
	APKR_PAQUET,
	APKR_ACTIVITE,
	APKR_LIBELLE,
	APKR_MEMOIRE,
	APKR_COUNT
}	t_apkr;

typedef struct s_apk
{
	t_span		file;
	t_zip		zip;
	t_zipent	manifest;
	t_zipent	dex;
	int			reason;
}	t_apk;

typedef struct s_apksig
{
	uint8_t		cert_sha256[32];
	uint32_t	algo;
	int			reason;
}	t_apksig;

typedef struct s_apkmanifest
{
	char		package[APK_PKG_MAX];
	char		version_name[APK_VNAME_MAX];
	char		label[APK_LABEL_MAX];
	char		activity[APK_CLASS_MAX];
	char		activity_desc[APK_DESC_MAX];
	char		perms[APK_PERM_MAX][APK_PERM_LEN];
	uint32_t	perm_count;
	uint32_t	version_code;
	uint32_t	min_sdk;
	uint32_t	target_sdk;
	int			reason;
}	t_apkmanifest;

int			apk_open(t_apk *a, t_span file);
int			apk_verify(const t_apk *a, t_apksig *out);
int			apk_manifest(const t_apk *a, t_apkmanifest *out);
int64_t		apk_read(const t_apk *a, const char *name, uint8_t **out);
const char	*apk_reason(int reason);

#endif
