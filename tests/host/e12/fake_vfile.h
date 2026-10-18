#ifndef FAKE_VFILE_H
# define FAKE_VFILE_H

# include <stddef.h>

void	fv_reset(void);
void	fv_write_budget(int bytes);
void	fv_read_budget(int bytes);
void	fv_rename_fail(int on);
void	fv_fsync_fail(int on);
void	fv_open_fail(int on);
int		fv_nfiles(void);
int		fv_plant(const char *path, const void *data, size_t len);
int		fv_slurp(const char *path, char *out, size_t max);

#endif
