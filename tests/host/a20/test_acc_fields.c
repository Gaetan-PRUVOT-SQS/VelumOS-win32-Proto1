#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "accounts.h"

static void	fields_decodes(void)
{
	t_account	a;
	const char	*l;

	l = "Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0";
	h_eq_i64("A valide", acc_parse_line(l, strlen(l), &a), 0);
	h_eq_str("nom", a.name, "Utilisateur");
	h_eq_u64("iterations", a.iterations, 10000);
	h_eq_u64("sel", a.salt_len, 16);
	h_eq_u64("sel octet 15", a.salt[15], 15);
	h_eq_u64("hash octet 0", a.hash[0], 0xe4);
	h_eq_u64("hash octet 31", a.hash[31], 0xc0);
	h_eq_u64("drapeaux", a.flags, 0);
	l = "Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2 ":1";
	h_eq_i64("desactive valide", acc_parse_line(l, strlen(l), &a), 0);
	h_eq_u64("drapeau desactive", a.flags, ACC_FLAG_DISABLED);
}

static void	fields_sortie_effacee_si_echec(void)
{
	t_account	a;
	const char	*bad;

	memset(&a, 0xaa, sizeof(a));
	bad = "U:9999:" V_SALT_A ":" V_HA1 V_HA2 ":0";
	h_eq_i64("refusee", acc_parse_line(bad, strlen(bad), &a), -22);
	h_eq_u64("nom efface", (uint8_t)a.name[0], 0);
	h_eq_u64("hash efface", a.hash[0], 0);
	h_eq_u64("iterations effacees", a.iterations, 0);
	h_eq_i64("trop longue", acc_parse_line(bad, ACC_LINE_MAX + 1, &a), -22);
}

static void	fields_decimal_et_hexa(void)
{
	t_field		f;
	uint32_t	v;
	uint8_t		raw[4];

	f.p = "4294967295";
	f.n = 10;
	h_eq_i64("u32 max", acc_dec(&f, &v), 0);
	h_eq_u64("u32 max valeur", v, 4294967295u);
	f.p = "4294967296";
	h_eq_i64("u32 + 1", acc_dec(&f, &v), -34);
	f.p = "0aFf";
	f.n = 4;
	h_eq_i64("hexa 2 octets", acc_hex(&f, raw, sizeof(raw)), 2);
	h_eq_u64("hexa octet 1", raw[1], 0xff);
	f.p = "00112233aa";
	f.n = 10;
	h_eq_i64("hexa trop long pour la sortie", acc_hex(&f, raw, 4), -22);
}

int	main(void)
{
	h_begin("a20/acc-fields");
	h_run("champs: decodes", fields_decodes);
	h_run("champs: sortie effacee si echec", fields_sortie_effacee_si_echec);
	h_run("champs: decimal et hexa", fields_decimal_et_hexa);
	return (h_end());
}
