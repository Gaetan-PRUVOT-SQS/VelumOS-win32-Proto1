#ifndef RUNCMD_H
# define RUNCMD_H

# include <stddef.h>

# define RUN_DIR "/system/bin/"
# define RUN_NAME_MAX 63
# define RUN_INPUT_MAX 256
# define RUN_APK_MAX 128

int	run_name_ok(const char *s, size_t n);
int	run_resolve(const char *input, size_t len, char *path, size_t size);
int	run_apk_path(const char *input, size_t len, char *path, size_t size);

#endif
