#include <stdint.h>
#include <stdio.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "accounts.h"

static size_t	put(char *buf, size_t pos, const char *s)
{
	size_t	n;

	n = strlen(s);
	memcpy(buf + pos, s, n);
	return (pos + n);
}

static void	file_tolerance_des_lignes_invalides(void)
{
	char		buf[1024];
	t_accounts	set;
	size_t		n;

	n = put(buf, 0, "# commentaire\n\ngarbage\r\n");
	n = put(buf, n, "Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0\r\n");
	n = put(buf, n, "x:y\n:::::\n");
	n = put(buf, n, "Alice:12000:" V_SALT_B ":" V_HB1 V_HB2 ":0");
	h_eq_i64("deux comptes", acc_parse_file(buf, n, &set), 2);
	h_eq_str("premier", set.list[0].name, "Utilisateur");
	h_eq_str("second sans fin de ligne", set.list[1].name, "Alice");
	h_eq_u64("iterations du second", set.list[1].iterations, 12000);
	h_eq_u64("count", set.count, 2);
}

static void	file_doublons_et_plafond(void)
{
	static char	buf[ACC_FILE_MAX];
	t_accounts	set;
	char		line[256];
	size_t		n;
	int			i;

	n = put(buf, 0, "U:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0\n");
	n = put(buf, n, "U:10000:" V_SALT_B ":" V_HB1 V_HB2 ":1\n");
	h_eq_i64("doublon ignore", acc_parse_file(buf, n, &set), 1);
	h_eq_u64("le premier gagne", set.list[0].flags, 0);
	n = 0;
	i = 0;
	while (i < 40)
	{
		snprintf(line, sizeof(line), "u%02d:10000:%s:%s%s:0\n", i, V_SALT_A,
			V_HA1, V_HA2);
		n = put(buf, n, line);
		i++;
	}
	h_eq_i64("plafond ACC_MAX", acc_parse_file(buf, n, &set), ACC_MAX);
	h_eq_str("dernier garde", set.list[ACC_MAX - 1].name, "u15");
}

static void	file_recherche_par_nom(void)
{
	char		buf[512];
	t_accounts	set;
	size_t		n;

	n = put(buf, 0, "Alice:12000:" V_SALT_B ":" V_HB1 V_HB2 ":0\n");
	acc_parse_file(buf, n, &set);
	h_true(acc_find(&set, "Alice") == &set.list[0], "trouve");
	h_true(acc_find(&set, "alice") == NULL, "casse differente");
	h_true(acc_find(&set, "Alic") == NULL, "prefixe");
	h_true(acc_find(&set, "Alice2") == NULL, "suffixe");
	h_true(acc_find(&set, "") == NULL, "vide");
}

int	main(void)
{
	h_begin("a20/acc-file");
	h_run("fichier: lignes invalides tolerees",
		file_tolerance_des_lignes_invalides);
	h_run("fichier: doublons et plafond", file_doublons_et_plafond);
	h_run("fichier: recherche par nom", file_recherche_par_nom);
	return (h_end());
}
