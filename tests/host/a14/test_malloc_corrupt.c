#include <stdint.h>
#include "alloc_int.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"
#include "string.h"

static void	corrupt_header(void)
{
	char	*p;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = malloc(64);
	memset(p - ALLOC_HDR, 0xee, ALLOC_HDR);
	free(p);
	h_eq_i64("une faute", fa_hook_count(), 1);
	h_eq_i64("code corruption", fa_hook_code(0), ALLOC_FAULT_CORRUPT);
}

static void	corrupt_class(void)
{
	void	*p;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = malloc(64);
	((t_ablock *)p - 1)->cls = 99;
	free(p);
	h_eq_i64("une faute", fa_hook_count(), 1);
	h_eq_i64("code corruption", fa_hook_code(0), ALLOC_FAULT_CORRUPT);
}

static void	corrupt_use_after_free(void)
{
	uint8_t	*p;
	uint8_t	*q;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = malloc(100);
	free(p);
	p[20] = 0;
	q = malloc(100);
	h_eq_i64("ecriture apres liberation detectee", fa_hook_count(), 1);
	h_eq_i64("code corruption", fa_hook_code(0), ALLOC_FAULT_CORRUPT);
	h_true(q != NULL && q != p, "nouveau bloc sain rendu");
	free(q);
}

static void	corrupt_beyond_window(void)
{
	uint8_t	*p;
	uint8_t	*q;

	fake_reset();
	fake_kernel_on();
	fa_hook_install();
	p = malloc(100);
	free(p);
	p[60] = 0;
	q = malloc(100);
	h_eq_i64("hors fenetre de controle : non detecte", fa_hook_count(), 0);
	h_true(q == p, "bloc reutilise");
	free(q);
}

int	main(void)
{
	h_begin("a14/malloc_corrupt");
	h_run("free/MC-DC : en-tete ecrase", corrupt_header);
	h_run("free/MC-DC : classe hors table", corrupt_class);
	h_run("malloc/erreur : ecriture apres liberation", corrupt_use_after_free);
	h_run("malloc/limite : au-dela de la fenetre", corrupt_beyond_window);
	return (h_end());
}
