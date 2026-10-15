#ifndef ZIP_H
# define ZIP_H

# include <stddef.h>
# include <stdint.h>
# include <velum/apk/apkdef.h>

# define ZIP_MAX_ENTRIES 4096
# define ZIP_NAME_MAX 255

typedef struct s_zip
{
	t_span		file;
	uint32_t	cd_off;
	uint32_t	cd_size;
	uint32_t	count;
}	t_zip;

typedef struct s_zipent
{
	const char	*name;
	uint32_t	name_len;
	uint32_t	method;
	uint32_t	crc;
	uint32_t	csize;
	uint32_t	usize;
	uint32_t	data_off;
	uint32_t	lfh_off;
}	t_zipent;

int			zip_open(t_zip *z, t_span file);
int			zip_entry(const t_zip *z, uint32_t index, t_zipent *out);
int			zip_find(const t_zip *z, const char *name, t_zipent *out);
int64_t		zip_extract(const t_zip *z, const t_zipent *e, uint8_t *out,
				size_t cap);
int			zip_name_safe(const char *name, size_t len);
int64_t		inflate_raw(t_span in, uint8_t *out, size_t cap);
uint32_t	zip_crc32(uint32_t crc, const uint8_t *p, size_t n);

#endif
