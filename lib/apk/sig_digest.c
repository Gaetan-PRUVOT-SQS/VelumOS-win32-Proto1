#include "apk_int.h"

static void	put32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

static void	section_hash(t_sha256 *top, t_span s)
{
	t_sha256	c;
	uint8_t		head[5];
	uint8_t		h[SHA256_LEN];
	size_t		n;

	while (s.len > 0)
	{
		n = s.len;
		if (n > APK_CHUNK)
			n = APK_CHUNK;
		head[0] = 0xa5;
		put32(head + 1, (uint32_t)n);
		sha256_init(&c);
		sha256_update(&c, head, 5);
		sha256_update(&c, s.p, n);
		sha256_final(&c, h);
		sha256_update(top, h, SHA256_LEN);
		s = (t_span){s.p + n, s.len - n};
	}
}

static void	eocd_hash(t_sha256 *top, t_span eocd, uint32_t block_off)
{
	t_sha256	c;
	uint8_t		head[5];
	uint8_t		h[SHA256_LEN];

	head[0] = 0xa5;
	put32(head + 1, (uint32_t)eocd.len);
	sha256_init(&c);
	sha256_update(&c, head, 5);
	sha256_update(&c, eocd.p, 16);
	put32(head, block_off);
	sha256_update(&c, head, 4);
	sha256_update(&c, eocd.p + 20, eocd.len - 20);
	sha256_final(&c, h);
	sha256_update(top, h, SHA256_LEN);
}

static uint32_t	chunk_count(size_t n)
{
	return ((uint32_t)((n + APK_CHUNK - 1) / APK_CHUNK));
}

void	sig_digest(const t_apk *a, const t_sigblock *b, uint8_t *out)
{
	t_sha256	top;
	uint8_t		head[5];
	t_span		f;

	f = a->file;
	head[0] = 0x5a;
	put32(head + 1, chunk_count(b->off) + chunk_count(a->zip.cd_size) + 1);
	sha256_init(&top);
	sha256_update(&top, head, 5);
	section_hash(&top, (t_span){f.p, b->off});
	section_hash(&top, (t_span){f.p + a->zip.cd_off, a->zip.cd_size});
	eocd_hash(&top, (t_span){f.p + b->eocd, f.len - b->eocd},
		(uint32_t)b->off);
	sha256_final(&top, out);
}
