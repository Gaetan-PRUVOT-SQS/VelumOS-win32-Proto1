#include "velum/libk.h"
#include "cpu_int.h"

static const char *const	g_exc[EXC_COUNT][2] = {
{"#DE", "erreur de division"}, {"#DB", "débogage"},
{"NMI", "interruption non masquable"}, {"#BP", "point d'arrêt"},
{"#OF", "débordement"}, {"#BR", "borne dépassée"},
{"#UD", "instruction invalide"}, {"#NM", "FPU indisponible"},
{"#DF", "double faute"}, {"#CSO", "segment coprocesseur"},
{"#TS", "TSS invalide"}, {"#NP", "segment absent"},
{"#SS", "faute de pile"}, {"#GP", "protection générale"},
{"#PF", "faute de page"}, {"#15", "réservé"},
{"#MF", "erreur x87"}, {"#AC", "alignement"},
{"#MC", "erreur machine"}, {"#XM", "erreur SIMD"},
{"#VE", "virtualisation"}, {"#CP", "protection de contrôle"},
{"#22", "réservé"}, {"#23", "réservé"}, {"#24", "réservé"},
{"#25", "réservé"}, {"#26", "réservé"}, {"#27", "réservé"},
{"#HV", "injection hyperviseur"}, {"#VC", "communication VMM"},
{"#SX", "sécurité"}, {"#31", "réservé"}
};

static const char *const	g_faults[FAULT_COUNT] = {
	"div0", "gp", "ud", "pf", "df", "stack", "nmi"
};

const char	*trap_name(uint64_t vec)
{
	if (vec >= EXC_COUNT)
		return ("IRQ");
	return (g_exc[vec][0]);
}

const char	*trap_desc(uint64_t vec)
{
	if (vec >= EXC_COUNT)
		return ("interruption");
	return (g_exc[vec][1]);
}

int	fault_parse(const char *word)
{
	int	i;

	if (!word)
		return (FAULT_NONE);
	i = 0;
	while (i < FAULT_COUNT)
	{
		if (!strcmp(word, g_faults[i]))
			return (i);
		i++;
	}
	return (FAULT_NONE);
}

const char	*fault_name(int fault)
{
	if (fault < 0 || fault >= FAULT_COUNT)
		return ("aucune");
	return (g_faults[fault]);
}
