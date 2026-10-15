#include "crypto_int.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "velum/util.h"

int	pbkdf2_validate(const t_pbkdf2 *p, size_t outlen)
{
	if (!p || !outlen)
		return (E_INVAL);
	if (p->iters < 1 || p->iters > PBKDF2_ITERS_MAX)
		return (E_INVAL);
	if ((!p->pw && p->pwlen) || (!p->salt && p->saltlen))
		return (E_INVAL);
	if ((outlen - 1) / SHA256_LEN >= PBKDF2_BLOCKS_MAX)
		return (E_INVAL);
	return (0);
}

static void	pbkdf2_first(const t_hmac *tmpl, const t_pbkdf2 *p, uint32_t idx,
		uint8_t u[SHA256_LEN])
{
	t_hmac	h;
	uint8_t	be[4];

	h = *tmpl;
	crypto_store_be32(be, idx);
	hmac_update(&h, p->salt, p->saltlen);
	hmac_update(&h, be, sizeof(be));
	hmac_final(&h, u);
}

static void	pbkdf2_xor(uint8_t t[SHA256_LEN], const uint8_t u[SHA256_LEN])
{
	uint32_t	i;

	i = 0;
	while (i < SHA256_LEN)
	{
		t[i] ^= u[i];
		i++;
	}
}

static void	pbkdf2_block(const t_hmac *tmpl, const t_pbkdf2 *p, uint32_t idx,
		uint8_t t[SHA256_LEN])
{
	t_hmac		h;
	uint8_t		u[SHA256_LEN];
	uint32_t	i;

	pbkdf2_first(tmpl, p, idx, u);
	memcpy(t, u, SHA256_LEN);
	i = 1;
	while (i < p->iters)
	{
		h = *tmpl;
		hmac_update(&h, u, SHA256_LEN);
		hmac_final(&h, u);
		pbkdf2_xor(t, u);
		i++;
	}
	secure_zero(u, sizeof(u));
}

int	pbkdf2_sha256(const t_pbkdf2 *p, uint8_t *out, size_t outlen)
{
	t_hmac		tmpl;
	uint8_t		t[SHA256_LEN];
	uint32_t	idx;
	size_t		done;
	int			rc;

	rc = pbkdf2_validate(p, outlen);
	if (rc < 0 || !out)
		return (E_INVAL);
	hmac_prepare(&tmpl, p->pw, p->pwlen);
	idx = 1;
	done = 0;
	while (done < outlen)
	{
		pbkdf2_block(&tmpl, p, idx, t);
		memcpy(out + done, t, min_u64(outlen - done, SHA256_LEN));
		done += min_u64(outlen - done, SHA256_LEN);
		idx++;
	}
	secure_zero(&tmpl, sizeof(tmpl));
	secure_zero(t, sizeof(t));
	return (0);
}
