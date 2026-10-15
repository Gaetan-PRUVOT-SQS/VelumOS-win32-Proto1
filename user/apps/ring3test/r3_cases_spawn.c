#include "r3.h"

static void	r3_spawn_lengths(t_r3 *t)
{
	uint64_t	self;
	uint64_t	len;

	self = (uint64_t)R3_SELF;
	len = strlen(R3_SELF);
	r3_check(t, "spawn chemin noyau", r3_spawn_raw(R3_HHDM, 8, 0, 0),
		E_FAULT);
	r3_check(t, "spawn longueur géante",
		r3_spawn_raw(self, 1ull << 40, 0, 0), E_INVAL);
	r3_check(t, "spawn longueur nulle", r3_spawn_raw(self, 0, 0, 0),
		E_INVAL);
	r3_check(t, "spawn args non terminés",
		r3_spawn_raw(self, len, (uint64_t)"abc", 3), E_INVAL);
	r3_check(t, "spawn args géants",
		r3_spawn_raw(self, len, (uint64_t)"abc", 1ull << 20), E_INVAL);
	r3_check(t, "spawn args noyau", r3_spawn_raw(self, len, R3_KPTR, 4),
		E_FAULT);
}

void	r3_suite_spawn(t_r3 *t)
{
	r3_spawn_lengths(t);
	r3_check(t, "spawn chemin relatif", v_spawn("bin/x", NULL, 0, 0),
		E_INVAL);
	r3_check(t, "spawn UTF-8 invalide", v_spawn("/\303\050", NULL, 0, 0),
		E_INVAL);
	r3_check(t, "spawn caractère de contrôle",
		v_spawn("/system/\nbin", NULL, 0, 0), E_INVAL);
	r3_check(t, "spawn drapeau inconnu",
		v_spawn(R3_SELF, "exit42", 7, 0x100), E_INVAL);
	r3_check(t, "spawn fichier absent",
		v_spawn("/system/bin/absent", NULL, 0, 0), E_NOENT);
}
