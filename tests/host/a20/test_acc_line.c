#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "accounts.h"

static const t_linerow	g_rows[] = {
{"Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", 0, "compte A"},
{"Alice:12000:" V_SALT_B ":" V_HB1 V_HB2 ":0", 0, "compte B"},
{"Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2 ":1", 0, "desactive"},
{"Utilisateur:010000:" V_SALT_A ":" V_HA1 V_HA2 ":0", 0, "zeros de tete"},
{"Utilisateur:1000000:" V_SALT_A ":" V_HA1 V_HA2 ":0", 0, "iterations max"},
{"Utilisateur:10000:" V_SALT_C1 V_SALT_C2 ":" V_HC1 V_HC2 ":0", 0, "sel 32"},
{"U:10000:" V_SALT_A ":E418C26F08C4729D239ABD46EB0B9655"
	"4467E23DFB06F2AE503F27E8793AF0C0:0", 0, "hexa majuscule"},
{"abcdefghijklmnopqrstuvwxyz01234:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", 0,
	"nom de 31 octets"},
{"\303\211l\303\250ve:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", 0, "nom UTF-8"},
{"Jean Pierre:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", 0, "espace interne"},
{"", -22, "ligne vide"},
{"Utilisateur", -22, "un champ"},
{"Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2, -22, "quatre champs"},
{"Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0:x", -22, "six champs"},
{":::::", -22, "champs vides"},
{":10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "nom vide"},
{"abcdefghijklmnopqrstuvwxyz012345:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0",
	-22, "nom de 32 octets"},
{" Bob:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "espace de tete"},
{"Bob :10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "espace de queue"},
{"B\001b:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "controle dans le nom"},
{"B\303(:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "nom UTF-8 invalide"}
};

static const t_linerow	g_numbers[] = {
{"U:9999:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations 9999"},
{"U:1000001:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations 1000001"},
{"U:4294967295:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations u32 max"},
{"U:4294967296:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations u32+1"},
{"U:99999999999:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "11 chiffres"},
{"U::" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations vides"},
{"U:abc:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations texte"},
{"U:-1:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations negatives"},
{"U:+10000:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations signees"},
{"U:10 000:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations espace"},
{"U:1e4:" V_SALT_A ":" V_HA1 V_HA2 ":0", -22, "iterations notation"},
{"U:10000:" V_SALT_A ":" V_HA1 V_HA2 ":2", -22, "drapeau inconnu"},
{"U:10000:" V_SALT_A ":" V_HA1 V_HA2 ":3", -22, "drapeaux 3"},
{"U:10000:" V_SALT_A ":" V_HA1 V_HA2 ":x", -22, "drapeau texte"},
{"U:10000:" V_SALT_A ":" V_HA1 V_HA2 ":", -22, "drapeau vide"},
{"U:10000:" V_SALT_A ":" V_HA1 V_HA2 ":4294967295", -22, "drapeaux max"}
};

static const t_linerow	g_secrets[] = {
{"U:10000::" V_HA1 V_HA2 ":0", -22, "sel vide"},
{"U:10000:abc:" V_HA1 V_HA2 ":0", -22, "sel impair"},
{"U:10000:0g0102030405060708090a0b0c0d0e0f:" V_HA1 V_HA2 ":0", -22,
	"sel non hexa"},
{"U:10000:00010203040506:" V_HA1 V_HA2 ":0", -22, "sel de 7 octets"},
{"U:10000:" V_SALT_C1 V_SALT_C2 "a5:" V_HA1 V_HA2 ":0", -22, "sel de 33"},
{"U:10000:" V_SALT_A ":" V_HA1 ":0", -22, "hash de 16 octets"},
{"U:10000:" V_SALT_A ":" V_HA1 V_HA2 "00:0", -22, "hash de 33 octets"},
{"U:10000:" V_SALT_A ":" V_HA1 V_HA2 "0:0", -22, "hash impair"},
{"U:10000:" V_SALT_A ":g418c26f08c4729d239abd46eb0b9655" V_HA2 ":0", -22,
	"hash non hexa"},
{"U:10000:" V_SALT_A "::0", -22, "hash vide"}
};

static void	run_rows(const t_linerow *rows, uint32_t n)
{
	t_account	a;
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		h_eq_i64(rows[i].what,
			acc_parse_line(rows[i].line, strlen(rows[i].line), &a),
			rows[i].want);
		i++;
	}
}

static void	line_partitions_nom_et_structure(void)
{
	run_rows(g_rows, sizeof(g_rows) / sizeof(g_rows[0]));
}

static void	line_limites_numeriques_et_secrets(void)
{
	run_rows(g_numbers, sizeof(g_numbers) / sizeof(g_numbers[0]));
	run_rows(g_secrets, sizeof(g_secrets) / sizeof(g_secrets[0]));
}

int	main(void)
{
	h_begin("a20/acc-line");
	h_run("ligne: nom et structure", line_partitions_nom_et_structure);
	h_run("ligne: limites numeriques, secrets",
		line_limites_numeriques_et_secrets);
	return (h_end());
}
