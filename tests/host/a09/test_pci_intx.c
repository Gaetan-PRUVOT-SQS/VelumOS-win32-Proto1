#include "a09_test.h"
#include "fake_fab.h"
#include "harness.h"
#include "pci_int.h"
#include "velum/err.h"

static const t_fabspec	g_dev = {0, 3, 0, 0x1234, 0x1111, 3, 0, 0};

static void	bit_pose_et_statut_intact(void)
{
	t_fabdev		*f;
	const t_pcidev	*d;

	fab_reset();
	f = fab_make(&g_dev);
	pci_scan_root(0, 0);
	d = pci_at(0);
	fab_put(f, 4, 2, 0x0107);
	fab_put(f, 6, 2, 0xf900);
	g_fab.nlog = 0;
	h_eq_i64("coupure", pci_intx_disable(d), 0);
	h_eq_u64("bit 10 pose, autres bits conserves", fab_get(f, 4, 2), 0x0507);
	h_eq_u64("une seule ecriture", g_fab.nlog, 1);
	h_eq_u64("ecriture au registre Command", g_fab.log[0].off, 4);
	h_eq_u64("sur 16 bits, Status hors de portee", g_fab.log[0].width, 2);
	h_eq_u64("valeur ecrite", g_fab.log[0].val, 0x0507);
	h_eq_u64("aucune ecriture sur Status", fab_writes_at(6, 8), 0);
	h_eq_u64("Status intact", fab_get(f, 6, 2), 0xf900);
	h_eq_i64("deja coupee", pci_intx_disable(d), 0);
	h_eq_u64("idempotent sans reecriture", g_fab.nlog, 1);
}

static void	arguments_refuses(void)
{
	const t_pcidev	*d;
	t_pcidev		stranger;

	fab_reset();
	fab_make(&g_dev);
	pci_scan_root(0, 0);
	d = pci_at(0);
	stranger = *d;
	g_fab.nlog = 0;
	h_eq_i64("NULL", pci_intx_disable(NULL), E_INVAL);
	h_eq_i64("appareil etranger", pci_intx_disable(&stranger), E_INVAL);
	h_eq_i64("entree hors du compte", pci_intx_disable(d + 1), E_INVAL);
	h_eq_u64("aucune ecriture emise", g_fab.nlog, 0);
}

static void	materiel_qui_refuse(void)
{
	t_fabdev		*f;
	const t_pcidev	*d;

	fab_reset();
	f = fab_make(&g_dev);
	pci_scan_root(0, 0);
	d = pci_at(0);
	f->cmd_ro = 1;
	h_eq_i64("bit non retenu", pci_intx_disable(d), E_NOTSUP);
	h_eq_u64("commande inchangee", fab_get(f, 4, 2) & 0x400, 0);
	g_fab.ndev = 0;
	g_fab.nlog = 0;
	h_eq_i64("appareil disparu", pci_intx_disable(d), E_NODEV);
	h_eq_u64("pas d'ecriture vers le vide", g_fab.nlog, 0);
	h_eq_i64("verrous equilibres", pci_state()->lock.ticket
		- pci_state()->lock.serving, 0);
}

int	main(void)
{
	h_begin("a09/pci_intx");
	h_run("intx/bit-10-pose-status-intact", bit_pose_et_statut_intact);
	h_run("intx/appareil-nul-ou-inconnu-refuse", arguments_refuses);
	h_run("intx/bit-non-retenu-et-appareil-absent", materiel_qui_refuse);
	return (h_end());
}
