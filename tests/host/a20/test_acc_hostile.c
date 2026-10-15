#include <stdint.h>
#include <stdlib.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "accounts.h"

static const char	g_valid[] = "Utilisateur:10000:" V_SALT_A ":" V_HA1
	V_HA2 ":0\n";

static void	hostile_ligne_geante(void)
{
	char		*buf;
	t_accounts	set;
	size_t		giant;

	giant = 60000;
	buf = malloc(ACC_FILE_MAX);
	h_true(buf != NULL, "allocation");
	if (!buf)
		return ;
	memset(buf, 'a', giant);
	buf[giant] = '\n';
	memcpy(buf + giant + 1, g_valid, strlen(g_valid));
	h_eq_i64("compte apres la ligne geante",
		acc_parse_file(buf, giant + 1 + strlen(g_valid), &set), 1);
	h_eq_str("compte lu", set.list[0].name, "Utilisateur");
	free(buf);
}

static void	hostile_dix_mille_lignes(void)
{
	char		*buf;
	t_accounts	set;
	size_t		i;
	size_t		n;

	buf = malloc(ACC_FILE_MAX);
	h_true(buf != NULL, "allocation");
	if (!buf)
		return ;
	i = 0;
	while (i < 10000)
	{
		memcpy(buf + 2 * i, "x\n", 2);
		i++;
	}
	n = 2 * i;
	memcpy(buf + n, g_valid, strlen(g_valid));
	h_eq_i64("dix mille lignes",
		acc_parse_file(buf, n + strlen(g_valid), &set), 1);
	free(buf);
}

static void	hostile_taille_du_fichier(void)
{
	char		*buf;
	t_accounts	set;

	buf = malloc(ACC_FILE_MAX + 1);
	h_true(buf != NULL, "allocation");
	if (!buf)
		return ;
	memset(buf, '\n', ACC_FILE_MAX + 1);
	h_eq_i64("pile au plafond", acc_parse_file(buf, ACC_FILE_MAX, &set), 0);
	memset(&set, 0xff, sizeof(set));
	h_eq_i64("au dessus du plafond",
		acc_parse_file(buf, ACC_FILE_MAX + 1, &set), -34);
	h_eq_u64("ensemble vide apres refus", set.count, 0);
	free(buf);
}

static void	hostile_pointeurs_nuls_et_vide(void)
{
	t_accounts	set;

	h_eq_i64("ensemble nul", acc_parse_file("a", 1, NULL), -22);
	h_eq_i64("texte nul avec taille", acc_parse_file(NULL, 5, &set), -22);
	h_eq_i64("texte nul sans taille", acc_parse_file(NULL, 0, &set), 0);
	memset(&set, 0xff, sizeof(set));
	h_eq_i64("vide", acc_parse_file("", 0, &set), 0);
	h_eq_u64("ensemble remis a zero", set.count, 0);
}

int	main(void)
{
	h_begin("a20/acc-hostile");
	h_run("hostile: ligne geante", hostile_ligne_geante);
	h_run("hostile: 10000 lignes", hostile_dix_mille_lignes);
	h_run("hostile: taille du fichier", hostile_taille_du_fichier);
	h_run("hostile: pointeurs nuls et vide", hostile_pointeurs_nuls_et_vide);
	return (h_end());
}
