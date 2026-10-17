import importlib.util
import json
import sys
from pathlib import Path

import pytest

RACINE = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(RACINE / "tools"))


def charger(nom, chemin):
    spec = importlib.util.spec_from_file_location(nom, chemin)
    module = importlib.util.module_from_spec(spec)
    sys.modules[nom] = module
    spec.loader.exec_module(module)
    return module


matrice = charger("matrice_configurations", RACINE / "tests" / "qemu" / "matrice_configurations.py")

try:
    import vtest
except FileNotFoundError:
    vtest = None

necessite_vtest = pytest.mark.skipif(vtest is None, reason="OVMF absent, vtest non importable")

AIDE_CPU = """Available CPUs:
  Haswell               (alias configured by machine type)
  Haswell-v1            Intel Core Processor (Haswell)
  host                  processor with all supported host features
  max                   Enables all features supported by the accelerator
  qemu64                (alias configured by machine type)
Recognized CPUID flags:
  avx2 invtsc pcid smap smep sse4.1 tsc-deadline umip x2apic xsave
  kvm_pv_eoi
"""

AIDE_MACHINES = """Supported machines are:
pc                   Standard PC (i440FX + PIIX, 1996) (alias of pc-i440fx-10.2)
q35                  Standard PC (Q35 + ICH9, 2009) (alias of pc-q35-10.2)
none                 empty machine
"""

AIDE_PROPRIETES = """pc-q35-10.2-machine options:
  hpet=<bool>            - Enable/disable high precision event timer emulation
  smm=<OnOffAuto>        - Enable SMM
"""

JOURNAL = """[    0.000] I VelumOS 0.1, build abc
[    0.000] I cpu: GenuineIntel 6/60/4 "Haswell", pa 40 va 48, fpu 832 o, nx smep xsave pat
[    0.000] I pmm: total=251092 libre=251092 meta=72 reserve=10920 Kio
[    0.000] I pmm: plage 0x100000-0xf5fa000 libre
[    0.000] I acpi: HPET @0xffe22b5 long. 56 rév. 1 oem BOCHS
[    0.000] I acpi: 4 CPU, LAPIC @0xfee00000, 1 IOAPIC, 5 redirections, 0 entrées MADT ignorées
[    0.000] I time: TSC annoncé 0 Hz, mesuré 2000387992 Hz
[    0.013] I time: horloge HPET à 100000000 Hz, minuterie LAPIC one-shot, sans tic périodique
[    1.041] I VelumOS boot ok
[    3.094] W syscall: appel inconnu 0xff
"""


@pytest.fixture
def catalogue():
    modeles, drapeaux = matrice.lire_aide_cpu(AIDE_CPU)
    return matrice.Catalogue(
        modeles,
        drapeaux,
        matrice.lire_aide_machines(AIDE_MACHINES),
        {"q35": matrice.lire_proprietes_machine(AIDE_PROPRIETES)},
    )


def test_enumeration_noms_uniques_et_base_presente():
    configs = matrice.enumerer()
    noms = [c.nom for c in configs]
    assert len(noms) == len(set(noms))
    assert "base" in noms
    base = next(c for c in configs if c.nom == "base")
    assert (base.accel, base.cpu, base.smp, base.memoire, base.machine) == (
        "tcg",
        "max",
        1,
        "256M",
        "q35",
    )


def test_chaque_niveau_de_chaque_dimension_est_joue():
    configs = matrice.enumerer()
    for cpu in matrice.MODELES_TCG:
        assert any(c.accel == "tcg" and c.cpu == cpu for c in configs), cpu
    for cpu in matrice.MODELES_KVM:
        assert any(c.accel == "kvm" and c.cpu == cpu for c in configs), cpu
    for smp in matrice.NOMBRES_CPU:
        assert any(c.smp == smp for c in configs), smp
    for memoire in matrice.MEMOIRES:
        assert any(c.memoire == memoire for c in configs), memoire
    for machine in matrice.MACHINES:
        assert any(c.machine == machine for c in configs), machine


def test_echantillonnage_reste_loin_du_produit_cartesien():
    assert len(matrice.enumerer()) < matrice.combinaisons_complet() // 10


def test_les_croisements_a_risque_sont_presents():
    croisements = {c.nom for c in matrice.enumerer() if c.dimension == "croisement"}
    assert "cpu-qemu64+machine-pc-hpet-off" in croisements
    assert "mem-3584m+machine-pc" in croisements
    assert "mem-8g+machine-pc" in croisements
    assert "smp-4+mem-16m" in croisements


def test_filtrage_par_motif_accelerateur_et_dimension():
    configs = matrice.enumerer()
    assert {c.nom for c in matrice.filtrer(configs, motif="haswell")} == {"cpu-tcg-haswell"}
    assert all(c.accel == "kvm" for c in matrice.filtrer(configs, accel="kvm"))
    assert {c.smp for c in matrice.filtrer(configs, dimension="smp")} == {2, 4}
    assert matrice.filtrer(configs, motif="n-existe-pas") == []


def test_slug_est_sur_pour_un_nom_de_dossier():
    assert matrice.slug("max,-smep") == "max-sans-smep"
    assert matrice.slug("host,migratable=off") == "host-migratable-off"
    assert matrice.slug("Skylake-Client") == "skylake-client"
    assert "/" not in matrice.slug("a/b,c")


def test_ligne_qemu_decrit_la_configuration():
    config = next(c for c in matrice.enumerer() if c.nom == "icount")
    ligne = config.ligne_qemu()
    assert "-accel tcg" in ligne and "-icount shift=0,sleep=off" in ligne


def test_lecture_de_l_aide_cpu_et_machines(catalogue):
    assert {"Haswell", "host", "max", "qemu64"} <= catalogue.modeles
    assert "kvm-pv-eoi" in catalogue.drapeaux
    assert catalogue.machines == {"pc", "q35", "none"}
    assert catalogue.proprietes_machine["q35"] == {"hpet", "smm"}


def test_drapeau_inconnu_est_signale(catalogue):
    assert matrice.problemes_cpu("max,-smep", catalogue) == []
    assert matrice.problemes_cpu("host,migratable=off", catalogue) == []
    assert matrice.problemes_cpu("max,-fonction-imaginaire", catalogue) != []
    assert matrice.problemes_cpu("Pentium5", catalogue) != []


def test_option_de_machine_inconnue_est_signalee(catalogue):
    assert matrice.problemes_machine("q35,hpet=off", catalogue) == []
    assert matrice.problemes_machine("q35,turbo=on", catalogue) != []
    assert matrice.problemes_machine("vax", catalogue) != []


def test_incompatibilites_de_configuration(catalogue):
    kvm = matrice.Configuration("k", "cpu", accel="kvm", cpu="host")
    assert matrice.problemes_configuration(kvm, catalogue, kvm_disponible=True) == []
    assert any("KVM" in p for p in matrice.problemes_configuration(kvm, catalogue, False))
    host_tcg = matrice.Configuration("h", "cpu", accel="tcg", cpu="host")
    assert any("exige KVM" in p for p in matrice.problemes_configuration(host_tcg, catalogue, True))
    icount_kvm = matrice.Configuration("i", "icount", accel="kvm", cpu="host", icount="shift=0")
    assert any("icount" in p for p in matrice.problemes_configuration(icount_kvm, catalogue, True))


def test_modele_qmp_depuis_la_specification():
    assert matrice.spec_vers_modele_qmp("max") == {"name": "max", "props": {}}
    assert matrice.spec_vers_modele_qmp("max,-smep,+pcid") == {
        "name": "max",
        "props": {"smep": False, "pcid": True},
    }
    assert matrice.spec_vers_modele_qmp("host,migratable=off")["props"] == {"migratable": False}


def test_requete_qmp_est_du_json_ligne_par_ligne():
    lignes = matrice.commande_qmp("Haswell,-smep").strip().splitlines()
    assert [json.loads(ligne)["execute"] for ligne in lignes] == [
        "qmp_capabilities",
        "query-cpu-model-expansion",
        "quit",
    ]


def test_reponse_qmp_extrait_les_protections():
    sortie = "\n".join(
        [
            json.dumps({"QMP": {}}),
            json.dumps({"return": {}}),
            json.dumps(
                {
                    "return": {
                        "model": {"name": "x", "props": {"smep": True, "smap": False, "nx": True}}
                    }
                }
            ),
        ]
    )
    assert matrice.lire_reponse_qmp(sortie) == {"smep": True, "smap": False, "nx": True}


def test_reponse_qmp_en_erreur_donne_none():
    erreur = json.dumps({"error": {"class": "GenericError", "desc": "Property not found"}})
    assert matrice.lire_reponse_qmp(erreur) is None
    assert matrice.lire_reponse_qmp("pas du json") is None


def test_lecture_des_observations_du_journal():
    obs = matrice.lire_observations(JOURNAL)
    assert obs["drapeaux"] == ["nx", "smep", "xsave", "pat"]
    assert (obs["tsc_annonce"], obs["tsc_mesure"]) == (0, 2000387992)
    assert (obs["horloge"], obs["horloge_hz"]) == ("HPET", 100000000)
    assert obs["boot_ok_s"] == pytest.approx(1.041)
    assert obs["pmm_total_kio"] == 251092 and obs["pmm_libre_kio"] == 251092
    assert obs["cpus_acpi"] == 4
    assert obs["table_hpet"] is True
    assert obs["avertissements_noyau"] == ["syscall: appel inconnu 0xff"]


def test_protection_annoncee_a_tort_est_un_ecart():
    obs = {"drapeaux": ["nx", "smep"]}
    config = matrice.Configuration("c", "cpu", cpu="max,-smep")
    attendues = {"nx": True, "smep": False, "smap": False}
    ecarts = matrice.verifier_protections(obs, config, attendues, "")
    assert any("smep" in e and "annoncé" in e for e in ecarts)


def test_protection_filtree_par_tcg_est_toleree_sinon_ecart():
    obs = {"drapeaux": ["nx"]}
    config = matrice.Configuration("c", "cpu", cpu="Haswell")
    attendues = {"nx": True, "pcid": True}
    avert = "warning: TCG doesn't support requested feature: CPUID[eax=01h].ECX.pcid [bit 17]"
    assert matrice.verifier_protections(obs, config, attendues, avert) == []
    assert matrice.verifier_protections(obs, config, attendues, "") != []


def test_desactivation_explicite_est_verifiee_meme_sans_oracle():
    obs = {"drapeaux": ["xsave"]}
    config = matrice.Configuration("c", "cpu", cpu="max,-xsave")
    assert matrice.verifier_protections(obs, config, {}, "") == [
        "xsave désactivé par -xsave mais annoncé"
    ]


def test_horloge_mesuree_nulle_est_un_ecart():
    assert matrice.verifier_horloges({"tsc_annonce": 0, "tsc_mesure": 5, "horloge": "HPET"}) == []
    assert matrice.verifier_horloges({"tsc_annonce": 7, "tsc_mesure": 0, "horloge": "TSC"}) != []
    assert matrice.verifier_horloges({}) != []


def test_memoire_valeurs_limites():
    config = matrice.Configuration("m", "memoire", memoire="256M")
    bon = {"pmm_total_kio": 251092, "pmm_libre_kio": 251092}
    assert matrice.verifier_memoire(bon, config) == []
    assert matrice.verifier_memoire({"pmm_total_kio": 262145, "pmm_libre_kio": 262145}, config)
    assert matrice.verifier_memoire({"pmm_total_kio": 100, "pmm_libre_kio": 100}, config)
    assert matrice.verifier_memoire({"pmm_total_kio": 251092, "pmm_libre_kio": 200}, config)
    assert matrice.verifier_memoire({}, config) == ["ligne pmm absente"]


def test_au_dela_de_4_gio_une_plage_haute_est_exigee():
    config = matrice.Configuration("m", "memoire", memoire="8G")
    obs = {"pmm_total_kio": 8 * 1024 * 1024 - 20000, "pmm_libre_kio": 8 * 1024 * 1024 - 20000}
    sans_haute = "pmm: plage 0x100000-0xdfffffff libre"
    avec_haute = sans_haute + "\npmm: plage 0x100000000-0x1ffffffff libre"
    assert any("4 Gio" in e for e in matrice.verifier_memoire(obs, config, sans_haute))
    assert matrice.verifier_memoire(obs, config, avec_haute) == []


def test_hpet_off_exige_table_absente_et_horloge_de_repli():
    off = matrice.Configuration("h", "machine", machine="q35,hpet=off")
    assert matrice.verifier_hpet({"table_hpet": False, "horloge": "TSC"}, off) == []
    assert matrice.verifier_hpet({"table_hpet": True, "horloge": "TSC"}, off) != []
    assert matrice.verifier_hpet({"table_hpet": False, "horloge": "HPET"}, off) != []


def test_hpet_actif_exige_la_table_acpi():
    actif = matrice.Configuration("h", "machine", machine="pc,hpet=on")
    assert matrice.verifier_hpet({"table_hpet": True, "horloge": "HPET"}, actif) == []
    assert matrice.verifier_hpet({"table_hpet": False, "horloge": "TSC"}, actif) != []
    assert matrice.hpet_demande("q35")


def test_taille_en_kio():
    assert matrice.taille_en_kio("16M") == 16 * 1024
    assert matrice.taille_en_kio("8G") == 8 * 1024 * 1024
    assert matrice.taille_en_kio("3584M") == 3584 * 1024


def test_smp_le_nombre_de_cpu_acpi_suit_le_parametre():
    config = matrice.Configuration("s", "smp", smp=4)
    assert matrice.verifier_smp({"cpus_acpi": 4}, config) == []
    assert matrice.verifier_smp({"cpus_acpi": 1}, config) != []
    assert matrice.verifier_smp({}, config) == []


@necessite_vtest
def test_reglages_depuis_l_environnement():
    env = {
        "VTEST_ACCEL": "tcg",
        "VTEST_CPU": "Haswell",
        "VTEST_SMP": "2",
        "VTEST_MEM": "128M",
        "VTEST_MACHINE": "q35,hpet=off",
        "VTEST_DEBUG": "1",
        "VTEST_ICOUNT": "1",
    }
    reglages = vtest.Reglages.depuis_env(env)
    assert reglages == vtest.Reglages(
        "tcg", "Haswell", "q35,hpet=off", 2, "128M", True, "shift=0,sleep=off"
    )
    assert vtest.Reglages.depuis_env({}) == vtest.Reglages()


@necessite_vtest
@pytest.mark.parametrize(
    "champ,valeur",
    [
        ("accel", "xen"),
        ("smp", 0),
        ("memoire", "256 M"),
        ("memoire", "0M"),
        ("machine", "q35 -S"),
        ("cpu", "max;rm"),
        ("icount", "shift=0 -S"),
    ],
)
def test_reglages_refuse_les_valeurs_dangereuses(champ, valeur):
    with pytest.raises(vtest.VMError):
        vtest.Reglages(**{champ: valeur})


@necessite_vtest
def test_par_dessus_ne_remplace_que_les_valeurs_donnees():
    base = vtest.Reglages(cpu="Haswell", smp=2, debug=True)
    fusion = base.par_dessus(vtest.Reglages(cpu="max", memoire="64M"))
    assert (fusion.cpu, fusion.smp, fusion.memoire, fusion.debug) == ("max", 2, "64M", True)


@necessite_vtest
def test_commande_par_defaut_inchangee_avec_kvm(monkeypatch):
    monkeypatch.setattr(vtest.os, "access", lambda chemin, mode: True)
    monkeypatch.delenv("VTEST_SANS_KVM", raising=False)
    commande = vtest.VM("d.img", "sortie", reglages=vtest.Reglages()).commande()
    assert commande[:10] == [
        "qemu-system-x86_64",
        "-machine",
        "q35",
        "-m",
        "256M",
        "-smp",
        "1",
        "-enable-kvm",
        "-cpu",
        "host",
    ]


@necessite_vtest
def test_commande_par_defaut_inchangee_sans_kvm(monkeypatch):
    monkeypatch.setattr(vtest.os, "access", lambda chemin, mode: False)
    commande = vtest.VM("d.img", "sortie", reglages=vtest.Reglages()).commande()
    assert commande[7:9] == ["-cpu", "max"]
    assert "-enable-kvm" not in commande and "-accel" not in commande


@necessite_vtest
def test_options_explicites_dans_la_commande(monkeypatch):
    monkeypatch.setattr(vtest.os, "access", lambda chemin, mode: True)
    reglages = vtest.Reglages(
        accel="tcg",
        cpu="Skylake-Client",
        machine="q35,hpet=off",
        smp=4,
        memoire="3584M",
        debug=True,
        icount="shift=0,sleep=off",
    )
    commande = vtest.VM("d.img", "sortie", nom="x", reglages=reglages).commande()
    texte = " ".join(commande)
    assert "-machine q35,hpet=off -m 3584M -smp 4 -accel tcg -cpu Skylake-Client" in texte
    assert "-icount shift=0,sleep=off" in texte
    assert "-d int,cpu_reset,guest_errors -D sortie/x.qemu-debug.log" in texte


@necessite_vtest
def test_debug_sans_accelerateur_force_tcg_et_ignore_avec_kvm(monkeypatch):
    monkeypatch.setattr(vtest.os, "access", lambda chemin, mode: True)
    implicite = vtest.VM("d.img", "s", reglages=vtest.Reglages(debug=True)).commande()
    assert "-enable-kvm" not in implicite and "-d" in implicite
    machine = vtest.VM("d.img", "s", reglages=vtest.Reglages(accel="kvm", debug=True))
    commande = machine.commande()
    assert "-d" not in commande
    assert machine.avertissements


@necessite_vtest
def test_incompatibilites_levent_configuration_incompatible(monkeypatch):
    monkeypatch.setattr(vtest.os, "access", lambda chemin, mode: False)
    with pytest.raises(vtest.ConfigurationIncompatible):
        vtest.VM("d.img", "s", reglages=vtest.Reglages(accel="kvm")).commande()
    monkeypatch.setattr(vtest.os, "access", lambda chemin, mode: True)
    with pytest.raises(vtest.ConfigurationIncompatible):
        vtest.VM("d.img", "s", reglages=vtest.Reglages(accel="tcg", cpu="host")).commande()
    with pytest.raises(vtest.ConfigurationIncompatible):
        vtest.VM("d.img", "s", reglages=vtest.Reglages(accel="kvm", icount="shift=0")).commande()


@necessite_vtest
def test_la_memoire_du_scenario_prime_sur_l_environnement():
    reglages = vtest.Reglages(memoire="128M", smp=2)
    assert vtest.VM("d.img", "s", memoire="4G", reglages=reglages).memoire == "4G"
    assert vtest.VM("d.img", "s", reglages=reglages).memoire == "128M"
    assert vtest.VM("d.img", "s", reglages=reglages).smp == 2
    assert vtest.VM("d.img", "s", reglages=vtest.Reglages()).memoire == "256M"


@necessite_vtest
def test_la_matrice_n_est_jouee_que_sur_demande():
    run_scenarios = charger("run_scenarios_test", RACINE / "tools" / "run_scenarios.py")
    ordinaires = {nom for nom, _ in run_scenarios.charger_scenarios("")}
    assert "skel_boot" in ordinaires
    assert not any(nom.startswith("matrice_") for nom in ordinaires)
    matricielles = {nom for nom, _ in run_scenarios.charger_scenarios("", matrice=True)}
    assert matricielles == {"matrice_demarrage"}
