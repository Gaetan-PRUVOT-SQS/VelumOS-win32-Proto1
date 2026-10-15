#include <stdio.h>
#include <string.h>
#include "a09_test.h"
#include "harness.h"
#include "velum/crypto.h"
#include "velum/util.h"

#define MAX_LEN 300
#define MAX_CHUNK 200

static int	split_matches(const uint8_t *msg, size_t len, size_t chunk)
{
	uint8_t		whole[SHA256_LEN];
	uint8_t		parts[SHA256_LEN];
	t_sha256	c;
	size_t		off;

	sha256(msg, len, whole);
	sha256_init(&c);
	off = 0;
	while (off < len)
	{
		sha256_update(&c, msg + off, min_u64(chunk, len - off));
		off += chunk;
	}
	sha256_final(&c, parts);
	return (!memcmp(whole, parts, SHA256_LEN));
}

static void	decoupage_equivalent_a_un_seul_appel(void)
{
	uint8_t	msg[MAX_LEN];
	size_t	len;
	size_t	chunk;
	int		bad;

	fill_pattern(msg, sizeof(msg), 20261005);
	bad = 0;
	len = 0;
	while (len <= MAX_LEN)
	{
		chunk = 1;
		while (chunk <= MAX_CHUNK)
		{
			if (!split_matches(msg, len, chunk) && !bad++)
				fprintf(stderr, "  premier ecart : longueur %zu morceaux %zu\n",
					len, chunk);
			chunk++;
		}
		len++;
	}
	h_eq_i64("decoupages divergents (graine 20261005)", bad, 0);
}

static void	million_de_a_octet_par_octet(void)
{
	t_sha256	c;
	uint8_t		out[SHA256_LEN];
	uint32_t	i;

	sha256_init(&c);
	i = 0;
	while (i < 1000000)
	{
		sha256_update(&c, "a", 1);
		i++;
	}
	sha256_final(&c, out);
	h_eq_hex("nist million de a", out, SHA256_LEN,
		"cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

int	main(void)
{
	h_begin("a09/sha256_split");
	h_run("sha256/decoupage-1-a-200-octets-longueurs-0-a-300",
		decoupage_equivalent_a_un_seul_appel);
	h_run("sha256/million-de-a-un-octet-par-appel",
		million_de_a_octet_par_octet);
	return (h_end());
}
