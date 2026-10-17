import importlib.util
import os
import sys
import time
from pathlib import Path

from vtest import Reglages, VMError

NAME = "matrice QEMU : démarrage, horloges et protections par configuration"
CMDLINE = "selftest exit"
FIRMWARES = ("bios",)
DELAI_DEMARRAGE = 180
DELAI_AUTOTESTS = 300


def _charger_outils():
    nom = "matrice_configurations"
    if nom in sys.modules:
        return sys.modules[nom]
    chemin = Path(__file__).resolve().with_name(f"{nom}.py")
    spec = importlib.util.spec_from_file_location(nom, chemin)
    module = importlib.util.module_from_spec(spec)
    sys.modules[nom] = module
    spec.loader.exec_module(module)
    return module


OUTILS = _charger_outils()
CONFIGURATIONS = OUTILS.enumerer()

_catalogue = None


class EchecMatrice(AssertionError):
    def __init__(self, message, observations):
        super().__init__(message)
        self.observations = observations


def kvm_disponible():
    return os.access("/dev/kvm", os.W_OK)


def catalogue():
    global _catalogue
    if _catalogue is None:
        machines = {c.machine.split(",")[0] for c in CONFIGURATIONS}
        _catalogue = OUTILS.charger_catalogue(sorted(machines))
    return _catalogue


def ignorer_si(configuration):
    problemes = OUTILS.problemes_configuration(configuration, catalogue(), kvm_disponible())
    return "; ".join(problemes) if problemes else None


def reglages(configuration):
    return Reglages(
        accel=configuration.accel,
        cpu=configuration.cpu,
        machine=configuration.machine,
        smp=configuration.smp,
        memoire=configuration.memoire,
        icount=configuration.icount,
    )


def _attendre_demarrage(vm, observations):
    debut = time.monotonic()
    vm.attendre(r"VelumOS boot ok", delai=DELAI_DEMARRAGE)
    observations["boot_ok_mur_s"] = round(time.monotonic() - debut, 2)
    verdict = vm.attendre(r"SELFTESTS (PASS|FAIL)", delai=DELAI_AUTOTESTS)
    if verdict.group(1) != "PASS":
        raise AssertionError("autotests du noyau en échec")


def _ecarts(configuration, observations, texte_serie, erreurs_qemu):
    outils = OUTILS
    ecarts = outils.verifier_horloges(observations)
    ecarts += outils.verifier_memoire(observations, configuration, texte_serie)
    ecarts += outils.verifier_smp(observations, configuration)
    ecarts += outils.verifier_hpet(observations, configuration)
    attendues = outils.protections_attendues(configuration)
    if attendues is None:
        observations["oracle_protections"] = "indisponible"
        return ecarts
    observations["oracle_protections"] = sorted(p for p, v in attendues.items() if v)
    ecarts += outils.verifier_protections(observations, configuration, attendues, erreurs_qemu)
    return ecarts


def run(vm, configuration):
    observations = {"configuration": configuration.nom, "ligne_qemu": configuration.ligne_qemu()}
    try:
        _attendre_demarrage(vm, observations)
    except (VMError, AssertionError) as erreur:
        observations.update(OUTILS.lire_observations(vm.serie()))
        raise EchecMatrice(str(erreur), observations) from erreur
    texte = vm.serie()
    observations.update(OUTILS.lire_observations(texte))
    erreurs_qemu = vm.erreurs_qemu()
    observations["avertissements_qemu"] = [
        ligne for ligne in erreurs_qemu.splitlines() if "warning" in ligne
    ]
    ecarts = _ecarts(configuration, observations, texte, erreurs_qemu)
    if "FAIL" in texte:
        ecarts.append("le journal contient FAIL")
    if ecarts:
        raise EchecMatrice("; ".join(ecarts), observations)
    return observations
