#include <string.h>
#include "a09_test.h"
#include "harness.h"
#include "velum/crypto.h"

static void	update_vide_neutre(void)
{
	t_sha256	c;
	uint8_t		out[SHA256_LEN];

	sha256_init(&c);
	sha256_update(&c, NULL, 0);
	sha256_update(&c, "abc", 0);
	sha256_update(&c, "abc", 3);
	sha256_update(&c, NULL, 0);
	sha256_final(&c, out);
	h_eq_hex("abc avec mises a jour vides", out, SHA256_LEN,
		"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	sha256(NULL, 0, out);
	h_eq_hex("message vide par pointeur nul", out, SHA256_LEN,
		"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

static void	contexte_efface_apres_final(void)
{
	t_sha256	c;
	t_sha256	zero;
	uint8_t		out[SHA256_LEN];

	memset(&zero, 0, sizeof(zero));
	sha256_init(&c);
	sha256_update(&c, "secret", 6);
	sha256_final(&c, out);
	h_true(!memcmp(&c, &zero, sizeof(c)), "contexte remis a zero apres final");
}

static void	reutilisation_apres_init(void)
{
	t_sha256	c;
	uint8_t		a[SHA256_LEN];
	uint8_t		b[SHA256_LEN];

	sha256_init(&c);
	sha256_update(&c, "premier message", 15);
	sha256_final(&c, a);
	sha256_init(&c);
	sha256_update(&c, "premier message", 15);
	sha256_final(&c, b);
	h_true(!memcmp(a, b, SHA256_LEN), "meme message, meme condense");
}

int	main(void)
{
	h_begin("a09/sha256_ctx");
	h_run("sha256/update-vide-et-pointeur-nul", update_vide_neutre);
	h_run("sha256/final-efface-le-contexte", contexte_efface_apres_final);
	h_run("sha256/reutilisation-apres-init", reutilisation_apres_init);
	return (h_end());
}
