#include "harness.h"
#include "velum/crypto.h"
#include "velum/err.h"
#include "d05_vec.h"

static void	vec_table(void)
{
	size_t	i;

	i = 0;
	while (i < g_vec_count)
	{
		h_true(vec_check(&g_vec[i]), g_vec[i].name);
		i++;
	}
	h_true(g_vec_count >= 40, "nombre de vecteurs");
}

static void	vec_digests(void)
{
	uint8_t	d[SHA256_LEN];
	size_t	i;

	i = 0;
	while (i < g_vec_count)
	{
		if (g_vec[i].msg_len)
		{
			sha256(g_vec[i].msg, g_vec[i].msg_len, d);
			h_true(ct_equal(d, g_vec[i].digest, SHA256_LEN), g_vec[i].name);
		}
		i++;
	}
}

static void	api_null(void)
{
	const t_vec	*v;
	t_rsapub	k;
	t_span		s;

	v = vec_find(VEC_BASE_SPKI);
	h_true(v != NULL, "vecteur de base");
	if (!v)
		return ;
	s.p = v->key;
	s.len = v->key_len;
	h_true(der_spki_rsa(s, NULL) < 0, "sortie nulle");
	h_true(x509_spki(s, NULL) < 0, "spki nul");
	h_eq_i64("cle valide", der_spki_rsa(s, &k), 0);
	s.p = NULL;
	h_true(der_spki_rsa(s, &k) < 0, "entree nulle");
	h_true(rsa_pkcs1_sha256_verify(&k, s, v->digest) < 0, "signature nulle");
	s.p = v->sig;
	s.len = v->sig_len;
	h_true(rsa_pkcs1_sha256_verify(NULL, s, v->digest) < 0, "cle nulle");
	h_true(rsa_pkcs1_sha256_verify(&k, s, NULL) < 0, "condensat nul");
	h_eq_i64("temoin positif", rsa_pkcs1_sha256_verify(&k, s, v->digest), 0);
}

static void	api_key_struct(void)
{
	const t_vec	*v;
	t_rsapub	k;
	t_rsapub	bad;
	t_span		s;

	v = vec_find(VEC_BASE_SPKI);
	if (!v)
		return ;
	s.p = v->key;
	s.len = v->key_len;
	h_eq_i64("cle valide", der_spki_rsa(s, &k), 0);
	s.p = v->sig;
	s.len = v->sig_len;
	bad = k;
	bad.e = 1;
	h_true(rsa_pkcs1_sha256_verify(&bad, s, v->digest) < 0, "e = 1 direct");
	bad = k;
	bad.nlen = RSA_MAX_BYTES + 1;
	h_true(rsa_pkcs1_sha256_verify(&bad, s, v->digest) < 0, "nlen 513");
	bad.nlen = 0;
	h_true(rsa_pkcs1_sha256_verify(&bad, s, v->digest) < 0, "nlen 0");
	bad = k;
	bad.n[bad.nlen - 1] &= 0xfe;
	h_true(rsa_pkcs1_sha256_verify(&bad, s, v->digest) < 0, "n pair direct");
}

int	main(void)
{
	h_begin("d05 vecteurs rsa");
	h_run("vec_table", vec_table);
	h_run("vec_digests", vec_digests);
	h_run("api_null", api_null);
	h_run("api_key_struct", api_key_struct);
	return (h_end());
}
