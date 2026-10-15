#ifndef RESPOOL_H
# define RESPOOL_H

# include "velum/apk/apkdef.h"

# define RES_STRING_POOL 0x0001
# define RES_TABLE 0x0002
# define RES_XML 0x0003
# define RES_XML_START_NS 0x0100
# define RES_XML_END_NS 0x0101
# define RES_XML_START 0x0102
# define RES_XML_END 0x0103
# define RES_XML_CDATA 0x0104
# define RES_XML_RESMAP 0x0180
# define RES_TABLE_PACKAGE 0x0200
# define RES_TABLE_TYPE 0x0201
# define RES_TABLE_TYPE_SPEC 0x0202
# define RES_POOL_UTF8 0x100
# define RES_POOL_HEADER 28
# define RES_SIZE_MAX 0x7fffffff

typedef struct s_reschunk
{
	uint32_t	off;
	uint32_t	type;
	uint32_t	hsize;
	uint32_t	size;
}	t_reschunk;

typedef struct s_respool
{
	const uint8_t	*p;
	uint32_t		size;
	uint32_t		hsize;
	uint32_t		count;
	uint32_t		strings;
	uint32_t		utf8;
}	t_respool;

int	respool_open(t_respool *p, t_span chunk);
int	respool_get(const t_respool *p, uint32_t index, t_text out);

#endif
