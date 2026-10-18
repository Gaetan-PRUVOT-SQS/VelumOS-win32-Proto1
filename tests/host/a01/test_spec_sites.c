#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"

static char	*lire_fichier(const char *chemin)
{
	FILE	*f;
	char	*buf;
	long	n;
	size_t	lus;

	f = fopen(chemin, "rb");
	if (!f)
		return (NULL);
	if (fseek(f, 0, SEEK_END) != 0)
	{
		fclose(f);
		return (NULL);
	}
	n = ftell(f);
	if (n < 0 || n > 65536 || fseek(f, 0, SEEK_SET) != 0)
	{
		fclose(f);
		return (NULL);
	}
	buf = malloc((size_t)n + 1);
	if (!buf)
	{
		fclose(f);
		return (NULL);
	}
	lus = fread(buf, 1, (size_t)n, f);
	fclose(f);
	if (lus != (size_t)n)
	{
		free(buf);
		return (NULL);
	}
	buf[n] = '\0';
	return (buf);
}

static int	bornes_accolades(const char *texte, const char *nom,
		const char **deb, const char **fin)
{
	const char	*p;
	int			niveau;

	p = strstr(texte, nom);
	if (!p)
		return (0);
	p = strchr(p, '{');
	if (!p)
		return (0);
	*deb = p;
	niveau = 0;
	while (*p)
	{
		if (*p == '{')
			niveau++;
		else if (*p == '}')
		{
			niveau--;
			if (niveau == 0)
			{
				*fin = p + 1;
				return (1);
			}
		}
		p++;
	}
	return (0);
}

static int	bornes_asm(const char *texte, const char *marque,
		const char *fin_marque, const char **deb, const char **fin)
{
	const char	*p;
	const char	*q;

	p = strstr(texte, marque);
	if (!p)
		return (0);
	q = strstr(p, fin_marque);
	if (!q)
		return (0);
	*deb = p;
	*fin = q;
	return (1);
}

static int	pos_tranche(const char *deb, const char *fin, const char *aiguille)
{
	size_t		n;
	const char	*p;

	n = strlen(aiguille);
	p = deb;
	while (p + n <= fin)
	{
		if (memcmp(p, aiguille, n) == 0)
			return ((int)(p - deb));
		p++;
	}
	return (-1);
}

static int	compte(const char *deb, const char *fin, const char *aiguille)
{
	int			n;
	size_t		len;
	const char	*p;

	n = 0;
	len = strlen(aiguille);
	p = deb;
	while (p + len <= fin)
	{
		if (memcmp(p, aiguille, len) == 0)
		{
			n++;
			p += len;
		}
		else
			p++;
	}
	return (n);
}

static void	ordre(const char *deb, const char *fin, const char *avant,
		const char *apres, const char *msg)
{
	int	a;
	int	b;

	a = pos_tranche(deb, fin, avant);
	b = pos_tranche(deb, fin, apres);
	h_true(a >= 0 && b > a, msg);
}

static void	ap_applique_spec(void)
{
	char		*texte;
	const char	*deb;
	const char	*fin;

	deb = NULL;
	fin = NULL;
	texte = lire_fichier("arch/x86_64/smp/smp_ap.c");
	h_true(texte != NULL, "smp_ap.c lisible");
	if (!texte)
		return ;
	h_true(bornes_accolades(texte, "smp_ap_main", &deb, &fin),
		"corps de smp_ap_main");
	if (deb && fin)
	{
		h_eq_i64("un seul cpu_spec_apply_local",
			compte(deb, fin, "cpu_spec_apply_local("), 1);
		ordre(deb, fin, "cpu_cr_setup(", "cpu_fpu_boot(", "cr avant fpu");
		ordre(deb, fin, "cpu_fpu_boot(", "cpu_spec_apply_local(",
			"fpu avant spec");
		ordre(deb, fin, "cpu_spec_apply_local(", "hlt", "spec avant hlt");
		h_true(pos_tranche(deb, fin, "#if") < 0, "pas de condition compile");
	}
	free(texte);
}

static void	entree_user(void)
{
	char		*texte;
	const char	*deb;
	const char	*fin;
	const char	*bout;

	deb = NULL;
	fin = NULL;
	texte = lire_fichier("arch/x86_64/switch.S");
	h_true(texte != NULL, "switch.S lisible");
	if (!texte)
		return ;
	bout = texte + strlen(texte);
	h_eq_i64("aucune verw dans switch.S", compte(texte, bout, "verw"), 0);
	h_true(bornes_asm(texte, "arch_enter_user:", ".size arch_enter_user",
			&deb, &fin), "corps arch_enter_user");
	if (deb && fin)
	{
		h_eq_i64("un seul appel",
			compte(deb, fin, "call cpu_spec_user_return"), 1);
		ordre(deb, fin, "mov rbx, rdx", "push rdi",
			"argument sauve avant la trame");
		ordre(deb, fin, "push rdi", "sub rsp, 8",
			"trame iretq avant l'alignement");
		ordre(deb, fin, "sub rsp, 8", "call cpu_spec_user_return",
			"appel aligne");
		ordre(deb, fin, "call cpu_spec_user_return", "add rsp, 8",
			"trame retrouvee apres l'appel");
		ordre(deb, fin, "add rsp, 8", "xor eax, eax",
			"effacement apres l'appel");
		ordre(deb, fin, "xor r15d, r15d", "mov rdi, rbx",
			"rdi relu apres les xor");
		ordre(deb, fin, "mov rdi, rbx", "xor ebx, ebx",
			"rbx efface en dernier");
		ordre(deb, fin, "call cpu_spec_user_return", "swapgs",
			"appel avant swapgs, gs encore noyau");
		ordre(deb, fin, "xor ebx, ebx", "swapgs",
			"effacement termine avant swapgs");
		ordre(deb, fin, "swapgs", "iretq", "iretq en dernier");
		h_true(pos_tranche(deb, fin, "xor edi, edi") < 0
			&& pos_tranche(deb, fin, "xor rdi, rdi") < 0,
			"rdi garde l'argument");
	}
	free(texte);
}

static void	verw_reste_garde(void)
{
	char		*texte;
	const char	*deb;
	const char	*fin;
	const char	*bout;

	deb = NULL;
	fin = NULL;
	texte = lire_fichier("arch/x86_64/cpu/spec_boot.c");
	h_true(texte != NULL, "spec_boot.c lisible");
	if (!texte)
		return ;
	bout = texte + strlen(texte);
	h_eq_i64("une seule verw", compte(texte, bout, "verw %0"), 1);
	h_true(bornes_accolades(texte, "cpu_spec_user_return", &deb, &fin),
		"corps cpu_spec_user_return");
	if (deb && fin)
	{
		ordre(deb, fin, "g_spec_verw_user", "verw %0",
			"verw seulement si le drapeau est arme");
		h_eq_i64("la verw est dans la fonction",
			compte(deb, fin, "verw %0"), 1);
	}
	free(texte);
}

int	main(void)
{
	h_begin("a01/spec_sites");
	h_run("ap applique spec", ap_applique_spec);
	h_run("entree utilisateur", entree_user);
	h_run("verw gardee", verw_reste_garde);
	return (h_end());
}
