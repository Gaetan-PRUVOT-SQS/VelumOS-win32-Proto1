#include <stdio.h>
#include <string.h>
#include "fake_sched.h"
#include "harness.h"

#define CHAIN_MAX 6

typedef struct s_chain
{
	const char	*label;
	const char	*name[CHAIN_MAX];
	int			len;
}	t_chain;

static const char	*g_known[] = {"vfs.quota", "mutex", "event", "sem", "tmr",
	"sig", "wait-reg", "wait-block", "proctab", "proc", "waitq", "runq",
	"ps2", "pci", "input_queue", "input_xlate", "chan", "ports", "handles",
	"grave", "random", "obj", "pci_ecam", "pci_cf8", "reapq"};

static const t_chain	g_chains[] = {
	{"sched : mutex, waitq, runq", {"mutex", "waitq", "runq"}, 3},
	{"sched : event, waitq", {"event", "waitq"}, 2},
	{"sched : sem, waitq", {"sem", "waitq"}, 2},
	{"minuterie : tmr, wait-reg, wait-block, waitq, runq",
		{"tmr", "wait-reg", "wait-block", "waitq", "runq"}, 5},
	{"minuterie : tmr, sig", {"tmr", "sig"}, 2},
	{"processus : proctab, proc", {"proctab", "proc"}, 2},
	{"pci : pci, pci_ecam", {"pci", "pci_ecam"}, 2},
	{"pci : pci, pci_cf8", {"pci", "pci_cf8"}, 2},
	{"entrées : ps2, input_xlate", {"ps2", "input_xlate"}, 2},
	{"entrées : ps2, input_queue", {"ps2", "input_queue"}, 2},
	{"quota : vfs.quota, runq", {"vfs.quota", "runq"}, 2},
	{"quota : vfs.quota, ps2 (interruption sous quota)",
		{"vfs.quota", "ps2"}, 2},
	{"entrées : ps2, input_queue, wait-reg, wait-block, waitq, runq",
		{"ps2", "input_queue", "wait-reg", "wait-block", "waitq", "runq"}, 6},
	{"attente : wait-block, waitq", {"wait-block", "waitq"}, 2},
};

static int	known_count(void)
{
	return ((int)(sizeof(g_known) / sizeof(g_known[0])));
}

static int	chain_count(void)
{
	return ((int)(sizeof(g_chains) / sizeof(g_chains[0])));
}

static void	classes_toutes_ordonnees(void)
{
	int	i;

	h_eq_i64("classes de l'oracle", known_count(), RANK_BUILTIN);
	i = 0;
	while (i < known_count())
	{
		h_true(lockdep_rank_of(g_known[i]) != 0, g_known[i]);
		i++;
	}
}

static void	chaine_acquise(const t_chain *c)
{
	t_spinlock	l[CHAIN_MAX];
	int			i;

	i = 0;
	while (i < c->len)
	{
		spin_init(&l[i], c->name[i]);
		spin_lock(&l[i]);
		i++;
	}
	h_eq_u64(c->label, lockdep_depth(), (uint64_t)c->len);
	while (i > 0)
		spin_unlock(&l[--i]);
}

static void	ordre_observe_accepte(void)
{
	int	i;

	i = 0;
	while (i < chain_count())
		chaine_acquise(&g_chains[i++]);
	h_eq_u64("pile vide", lockdep_depth(), 0);
}

static void	inversion_detectee(const t_chain *c, int avant, int apres)
{
	t_spinlock	outer;
	t_spinlock	inner;
	char		attendu[96];
	char		etiquette[128];

	spin_init(&outer, c->name[apres]);
	spin_init(&inner, c->name[avant]);
	snprintf(etiquette, sizeof(etiquette), "%s : %s sous %s", c->label,
		c->name[avant], c->name[apres]);
	spin_lock(&outer);
	if (setjmp(*fake_panic_arm()) == 0)
	{
		spin_lock(&inner);
		fake_panic_disarm();
		h_true(0, etiquette);
	}
	else
	{
		snprintf(attendu, sizeof(attendu), "'%s' (rang %u) pris sous '%s'",
			c->name[avant], lockdep_rank_of(c->name[avant]),
			c->name[apres]);
		h_true(strstr(fake_panic_msg(), attendu) != NULL, etiquette);
	}
	fake_reset_self();
}

static void	ordre_inverse_refuse(void)
{
	int	i;
	int	avant;
	int	apres;

	i = 0;
	while (i < chain_count())
	{
		avant = 0;
		while (avant < g_chains[i].len)
		{
			apres = avant + 1;
			while (apres < g_chains[i].len)
				inversion_detectee(&g_chains[i], avant, apres++);
			avant++;
		}
		i++;
	}
}

static void	meme_classe_une(const char *name)
{
	t_spinlock	a;
	t_spinlock	b;

	spin_init(&a, name);
	spin_init(&b, name);
	spin_lock(&a);
	if (setjmp(*fake_panic_arm()) == 0)
	{
		spin_lock(&b);
		fake_panic_disarm();
		h_true(0, name);
	}
	else
		h_true(strstr(fake_panic_msg(), "pris sous") != NULL, name);
	fake_reset_self();
}

static void	meme_classe_refusee(void)
{
	int	i;

	i = 0;
	while (i < known_count())
		meme_classe_une(g_known[i++]);
}

int	main(void)
{
	h_begin("a06/lockdep_ordre");
	h_run("toutes les classes du noyau ont un rang", classes_toutes_ordonnees);
	h_run("ordre d'acquisition observé accepté", ordre_observe_accepte);
	h_run("ordre inversé refusé", ordre_inverse_refuse);
	h_run("même classe imbriquée refusée", meme_classe_refusee);
	return (h_end());
}
