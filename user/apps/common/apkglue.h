#ifndef APKGLUE_H
# define APKGLUE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/apk/apk.h"
# include "velum/apk/pm.h"

# define APKGLUE_FILE_MAX 0x4000000
# define APKGLUE_PATH_MAX 256
# define APKGLUE_CHUNK 0x10000
# define APKGLUE_OLD ".old"

int64_t			apkglue_read_file(const char *path, uint8_t **out);
size_t			apkglue_label(char *dst, size_t cap, const char *src);
int				apkglue_inspect(t_span apk, t_pminfo *out);
int				apkglue_reason(void);
const t_pmfs	*apkglue_fs(void);
int				apkglue_old_path(char *dst, const char *path);
int64_t			apkglue_fs_read(void *ctx, const char *path, t_text out);
int				apkglue_fs_write_new(void *ctx, const char *path, t_span data);

#endif
