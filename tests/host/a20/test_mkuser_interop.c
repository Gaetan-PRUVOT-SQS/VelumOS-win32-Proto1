#include <stdint.h>
#include <stdio.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "accounts.h"

static long	slurp(const char *path, char *buf, size_t max)
{
	FILE	*f;
	size_t	n;

	f = fopen(path, "rb");
	if (!f)
		return (-1);
	n = fread(buf, 1, max, f);
	fclose(f);
	return ((long)n);
}

static void	interop_compte_sans_mot_de_passe(void)
{
	static char	buf[ACC_FILE_MAX];
	t_accounts	set;
	long		n;

	n = slurp(A20_FIXTURE, buf, sizeof(buf));
	h_true(n > 0, "fixture lisible (make host-a20 la fabrique)");
	if (n <= 0)
		return ;
	h_eq_i64("un compte", acc_parse_file(buf, (size_t)n, &set), 1);
	h_eq_str("nom par defaut", set.list[0].name, "Utilisateur");
	h_eq_u64("iterations", set.list[0].iterations, ACC_ITER_MIN);
	h_eq_u64("sel de 16 octets", set.list[0].salt_len, 16);
	h_eq_u64("sel fixe: dernier octet", set.list[0].salt[15], 15);
	h_eq_i64("connexion par clic", acc_verify(&set.list[0], "", 0), 1);
	h_eq_i64("pas de mot de passe requis", acc_needs_password(&set.list[0]), 0);
	h_eq_i64("autre mot de passe refuse", acc_verify(&set.list[0], "x", 1), 0);
	h_true(acc_find(&set, "Utilisateur") == &set.list[0], "recherche par nom");
}

static void	interop_compte_avec_mot_de_passe(void)
{
	static char	buf[ACC_FILE_MAX];
	t_accounts	set;
	long		n;

	n = slurp(A20_FIXTURE_PW, buf, sizeof(buf));
	h_true(n > 0, "fixture avec mot de passe lisible");
	if (n <= 0)
		return ;
	h_eq_i64("un compte", acc_parse_file(buf, (size_t)n, &set), 1);
	h_eq_str("nom", set.list[0].name, "Alice");
	h_eq_u64("iterations", set.list[0].iterations, 12000);
	h_eq_i64("bon mot de passe", acc_verify(&set.list[0], "Passe 123", 9), 1);
	h_eq_i64("sans retour a la ligne", acc_verify(&set.list[0], "Passe 123\n",
			10), 0);
	h_eq_i64("mauvais", acc_verify(&set.list[0], "Passe 124", 9), 0);
	h_eq_i64("vide refuse", acc_verify(&set.list[0], "", 0), 0);
	h_eq_i64("mot de passe requis", acc_needs_password(&set.list[0]), 1);
}

int	main(void)
{
	h_begin("a20/mkuser-interop");
	h_run("interop: compte par defaut", interop_compte_sans_mot_de_passe);
	h_run("interop: avec mot de passe", interop_compte_avec_mot_de_passe);
	return (h_end());
}
