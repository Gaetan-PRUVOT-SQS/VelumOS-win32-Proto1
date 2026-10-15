#include <string.h>
#include "harness.h"
#include "help.h"

static void	header_lengths(void)
{
	t_wmhello	m;
	uint8_t		big[sizeof(t_wmhello) + 8];

	hv_hello(&m);
	h_eq_i64("R1 hello valide", hv_check(&m, sizeof(m)), 0);
	h_eq_i64("R2 tampon nul", wsp_validate(NULL, sizeof(m), 0), E_PROTO);
	h_eq_i64("R3 handle joint", wsp_validate(&m, sizeof(m), 1), E_PROTO);
	h_eq_i64("R4 longueur 0", hv_check(&m, 0), E_PROTO);
	h_eq_i64("R4 en-tete - 1", hv_check(&m, sizeof(t_wmhdr) - 1), E_PROTO);
	h_eq_i64("R5 > IPC_MSG_MAX", wsp_validate(&m, IPC_MSG_MAX + 1, 0),
		E_PROTO);
	m.h.size++;
	h_eq_i64("R7 taille annoncee", hv_check(&m, sizeof(m)), E_PROTO);
	memset(big, 0, sizeof(big));
	memcpy(big, &m, sizeof(m));
	h_eq_i64("R10 longueur + 1", hv_check(big, sizeof(m) + 1), E_PROTO);
}

static void	header_fields(void)
{
	t_wmhello	m;

	hv_hello(&m);
	m.h.magic ^= 1;
	h_eq_i64("R6 magic", hv_check(&m, sizeof(m)), E_PROTO);
	hv_hello(&m);
	m.h.status = -1;
	h_eq_i64("R8 statut", hv_check(&m, sizeof(m)), E_PROTO);
	hv_hello(&m);
	m.h.window = 5;
	h_eq_i64("R11 hello avec fenetre", hv_check(&m, sizeof(m)), E_PROTO);
	hv_hello(&m);
	m.version = 2;
	h_eq_i64("R13 version", hv_check(&m, sizeof(m)), E_PROTO);
}

static void	types(void)
{
	t_wmarg		a;
	uint32_t	t;
	int			bad;

	bad = 0;
	t = 0;
	while (t < 0x20)
	{
		hr_arg(&a, t, 1, 0);
		a.h.size = sizeof(t_wmhdr);
		if (t == 0 || t > WMC_CAPTURE)
			bad += hv_check(&a, sizeof(t_wmhdr)) != E_PROTO;
		t++;
	}
	h_eq_i64("R9 types inconnus refuses", bad, 0);
	hr_arg(&a, WMS_KEY, 1, 0);
	h_eq_i64("R9 type serveur refuse", hv_check(&a, sizeof(a)), E_PROTO);
	hr_arg(&a, WMC_DESTROY, 0, 0);
	a.h.size = sizeof(t_wmhdr);
	h_eq_i64("R12 fenetre nulle", hv_check(&a, sizeof(t_wmhdr)), E_PROTO);
	a.h.window = 3;
	h_eq_i64("destroy valide", hv_check(&a, sizeof(t_wmhdr)), 0);
}

static void	arguments(void)
{
	t_wmarg	a;

	hr_arg(&a, WMC_SET_STATE, 3, WSTATE_HIDDEN);
	h_eq_i64("etat limite", hv_check(&a, sizeof(a)), 0);
	a.value = WSTATE_HIDDEN + 1;
	h_eq_i64("R16 etat + 1", hv_check(&a, sizeof(a)), E_PROTO);
	hr_arg(&a, WMC_SET_CURSOR, 3, CUR_HAND + 1);
	h_eq_i64("R16 curseur", hv_check(&a, sizeof(a)), E_PROTO);
	hr_arg(&a, WMC_SET_ICON, 3, ICON_IDS);
	h_eq_i64("R16 icone", hv_check(&a, sizeof(a)), E_PROTO);
	hr_arg(&a, WMC_SET_ICON, 3, ICON_IDS - 1);
	h_eq_i64("icone limite", hv_check(&a, sizeof(a)), 0);
	hr_arg(&a, WMC_SUBSCRIBE, 0, 2);
	h_eq_i64("R16 abonnement", hv_check(&a, sizeof(a)), E_PROTO);
	hr_arg(&a, WMC_CAPTURE, 3, 1);
	a.reserved = 1;
	h_eq_i64("R16 reserve non nul", hv_check(&a, sizeof(a)), E_PROTO);
}

int	main(void)
{
	h_begin("a18/core_proto");
	h_run("en-tete longueurs", header_lengths);
	h_run("en-tete champs", header_fields);
	h_run("types", types);
	h_run("arguments", arguments);
	return (h_end());
}
