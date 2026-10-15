import dataclasses
import json
import re
import subprocess

QEMU = "qemu-system-x86_64"

PROTECTIONS = ("nx", "smep", "smap", "umip", "pcid", "xsave")

CLE_QMP = {
    "nx": "nx",
    "smep": "smep",
    "smap": "smap",
    "umip": "umip",
    "pcid": "pcid",
    "xsave": "xsave",
}

PROPRIETES_CPU_CONNUES = frozenset(
    {"migratable", "host-cache-info", "vendor", "family", "model", "stepping", "level", "model-id"}
)

MODELES_TCG = (
    "qemu64",
    "Nehalem",
    "Haswell",
    "Skylake-Client",
    "EPYC",
    "max",
    "max,-smep",
    "max,-smap",
    "max,-umip",
    "max,-xsave",
)
MODELES_KVM = ("host", "host,migratable=off", "host,-xsave")
NOMBRES_CPU = (1, 2, 4)
MEMOIRES = ("16M", "32M", "64M", "128M", "3584M", "8G")
MACHINES = ("pc,hpet=off", "pc,hpet=on", "q35,hpet=off", "q35,hpet=on")

BASE_ACCEL = "tcg"
BASE_CPU = "max"
BASE_SMP = 1
BASE_MEMOIRE = "256M"
BASE_MACHINE = "q35"
ICOUNT = "shift=0,sleep=off"
MARGE_NOYAU_KIO = 16 * 1024
SEUIL_4GIO_KIO = 4 * 1024 * 1024


@dataclasses.dataclass(frozen=True)
class Configuration:
    nom: str
    dimension: str
    accel: str = BASE_ACCEL
    cpu: str = BASE_CPU
    smp: int = BASE_SMP
    memoire: str = BASE_MEMOIRE
    machine: str = BASE_MACHINE
    icount: str | None = None

    def ligne_qemu(self):
        morceaux = [f"-accel {self.accel}", f"-cpu {self.cpu}", f"-machine {self.machine}"]
        morceaux += [f"-m {self.memoire}", f"-smp {self.smp}"]
        if self.icount:
            morceaux.append(f"-icount {self.icount}")
        return " ".join(morceaux)

    def modele_cpu(self):
        return self.cpu.split(",")[0]


def slug(valeur):
    texte = valeur.lower().replace(",-", "-sans-").replace(",+", "-avec-")
    texte = re.sub(r"[^a-z0-9]+", "-", texte)
    return texte.strip("-")


def _nom(*parties):
    return "+".join(parties)


def _cpu_nom(accel, cpu):
    return f"cpu-{accel}-{slug(cpu)}"


def _variations_seules():
    configs = [Configuration("base", "base")]
    for cpu in MODELES_TCG:
        if cpu != BASE_CPU:
            configs.append(Configuration(_cpu_nom("tcg", cpu), "cpu", cpu=cpu))
    for cpu in MODELES_KVM:
        configs.append(Configuration(_cpu_nom("kvm", cpu), "cpu", accel="kvm", cpu=cpu))
    for smp in NOMBRES_CPU:
        if smp != BASE_SMP:
            configs.append(Configuration(f"smp-{smp}", "smp", smp=smp))
    for memoire in MEMOIRES:
        configs.append(Configuration(f"mem-{memoire.lower()}", "memoire", memoire=memoire))
    for machine in MACHINES:
        configs.append(Configuration(f"machine-{slug(machine)}", "machine", machine=machine))
    configs.append(Configuration("icount", "icount", icount=ICOUNT))
    return configs


def _croisements():
    pc_off = "pc,hpet=off"
    q35_off = "q35,hpet=off"
    return [
        Configuration(
            _nom("cpu-qemu64", "machine-pc-hpet-off"), "croisement", cpu="qemu64", machine=pc_off
        ),
        Configuration(_nom("cpu-qemu64", "mem-16m"), "croisement", cpu="qemu64", memoire="16M"),
        Configuration(_nom("smp-4", "mem-16m"), "croisement", smp=4, memoire="16M"),
        Configuration(_nom("smp-4", "machine-q35-hpet-off"), "croisement", smp=4, machine=q35_off),
        Configuration(_nom("mem-3584m", "machine-pc"), "croisement", memoire="3584M", machine="pc"),
        Configuration(_nom("mem-8g", "machine-pc"), "croisement", memoire="8G", machine="pc"),
        Configuration(
            _nom("icount", "machine-q35-hpet-off"), "croisement", machine=q35_off, icount=ICOUNT
        ),
        Configuration(_nom("cpu-kvm-host", "smp-4"), "croisement", accel="kvm", cpu="host", smp=4),
        Configuration(
            _nom("cpu-kvm-host", "mem-8g"), "croisement", accel="kvm", cpu="host", memoire="8G"
        ),
        Configuration(
            _nom("cpu-kvm-host", "machine-pc-hpet-off"),
            "croisement",
            accel="kvm",
            cpu="host",
            machine=pc_off,
        ),
    ]


def enumerer():
    configs = _variations_seules() + _croisements()
    noms = [c.nom for c in configs]
    doublons = {n for n in noms if noms.count(n) > 1}
    if doublons:
        raise ValueError(f"noms de configuration en double : {sorted(doublons)}")
    return configs


def filtrer(configs, motif="", accel=None, dimension=None):
    retenues = []
    for config in configs:
        if motif and motif not in config.nom:
            continue
        if accel and config.accel != accel:
            continue
        if dimension and config.dimension != dimension:
            continue
        retenues.append(config)
    return retenues


def combinaisons_complet():
    return len(MODELES_TCG) * len(NOMBRES_CPU) * len(MEMOIRES) * len(MACHINES) + (
        len(MODELES_KVM) * len(NOMBRES_CPU) * len(MEMOIRES) * len(MACHINES)
    )


@dataclasses.dataclass
class Catalogue:
    modeles: frozenset
    drapeaux: frozenset
    machines: frozenset
    proprietes_machine: dict


def normaliser(nom):
    return nom.replace("_", "-")


def lire_aide_cpu(texte):
    modeles, drapeaux = set(), set()
    dans_drapeaux = False
    for ligne in texte.splitlines():
        if ligne.startswith("Recognized CPUID flags"):
            dans_drapeaux = True
            continue
        if dans_drapeaux:
            drapeaux.update(normaliser(mot) for mot in ligne.split())
        elif ligne.startswith("  ") and ligne.strip():
            modeles.add(ligne.split()[0])
    return frozenset(modeles), frozenset(drapeaux)


def lire_aide_machines(texte):
    machines = set()
    for ligne in texte.splitlines():
        champs = ligne.split()
        if len(champs) >= 2 and not ligne.startswith(("Supported", " ")):
            machines.add(champs[0])
    return frozenset(machines)


def lire_proprietes_machine(texte):
    return frozenset(
        m.group(1) for m in re.finditer(r"^\s+([a-z0-9-]+)=<", texte, flags=re.MULTILINE)
    )


def executer_texte(arguments, executer=subprocess.run):
    resultat = executer(arguments, capture_output=True, text=True, timeout=30, check=False)
    return resultat.stdout + resultat.stderr


def charger_catalogue(machines_utilisees, executer=subprocess.run):
    modeles, drapeaux = lire_aide_cpu(executer_texte([QEMU, "-cpu", "help"], executer))
    machines = lire_aide_machines(executer_texte([QEMU, "-machine", "help"], executer))
    proprietes = {}
    for nom in machines_utilisees:
        texte = executer_texte([QEMU, "-machine", f"{nom},help"], executer)
        proprietes[nom] = lire_proprietes_machine(texte)
    return Catalogue(modeles, drapeaux, machines, proprietes)


def problemes_cpu(spec, catalogue):
    problemes = []
    modele, *options = spec.split(",")
    if modele not in catalogue.modeles:
        problemes.append(f"modèle de CPU inconnu : {modele}")
    for option in options:
        if option[:1] in "+-" and len(option) > 1:
            nom = option[1:]
        else:
            nom = option.split("=", 1)[0]
        if normaliser(nom) not in catalogue.drapeaux and nom not in PROPRIETES_CPU_CONNUES:
            problemes.append(f"drapeau CPU inconnu : {nom}")
    return problemes


def problemes_machine(spec, catalogue):
    problemes = []
    nom, *options = spec.split(",")
    if nom not in catalogue.machines:
        problemes.append(f"machine inconnue : {nom}")
        return problemes
    connues = catalogue.proprietes_machine.get(nom)
    if connues is None:
        return problemes
    for option in options:
        cle = option.split("=", 1)[0]
        if cle not in connues:
            problemes.append(f"option de machine inconnue pour {nom} : {cle}")
    return problemes


def problemes_configuration(config, catalogue, kvm_disponible):
    problemes = problemes_cpu(config.cpu, catalogue) + problemes_machine(config.machine, catalogue)
    if config.accel == "kvm" and not kvm_disponible:
        problemes.append("KVM indisponible (/dev/kvm inaccessible)")
    if config.accel == "tcg" and config.modele_cpu() == "host":
        problemes.append("-cpu host exige KVM")
    if config.icount and config.accel != "tcg":
        problemes.append("-icount exige TCG")
    return problemes


def spec_vers_modele_qmp(spec):
    nom, *options = spec.split(",")
    props = {}
    for option in options:
        if option[:1] in "+-" and len(option) > 1:
            props[option[1:]] = option[0] == "+"
        elif "=" in option:
            cle, valeur = option.split("=", 1)
            if valeur in ("on", "true"):
                props[cle] = True
            elif valeur in ("off", "false"):
                props[cle] = False
            else:
                props[cle] = valeur
        else:
            props[option] = True
    return {"name": nom, "props": props}


def commande_qmp(spec):
    requete = [
        {"execute": "qmp_capabilities"},
        {
            "execute": "query-cpu-model-expansion",
            "arguments": {"type": "full", "model": spec_vers_modele_qmp(spec)},
        },
        {"execute": "quit"},
    ]
    return "\n".join(json.dumps(ligne) for ligne in requete) + "\n"


def lire_reponse_qmp(texte):
    for ligne in texte.splitlines():
        try:
            objet = json.loads(ligne)
        except ValueError:
            continue
        if "error" in objet:
            return None
        retour = objet.get("return")
        if isinstance(retour, dict) and "model" in retour:
            props = retour["model"].get("props", {})
            return {p: bool(props[CLE_QMP[p]]) for p in PROTECTIONS if CLE_QMP[p] in props}
    return None


def protections_attendues(config, executer=subprocess.run):
    arguments = [QEMU, "-machine", "none", "-nodefaults", "-display", "none", "-qmp", "stdio"]
    arguments += ["-accel", config.accel]
    resultat = executer(
        arguments,
        input=commande_qmp(config.cpu),
        capture_output=True,
        text=True,
        timeout=30,
        check=False,
    )
    return lire_reponse_qmp(resultat.stdout)


MOTIF_CPU = re.compile(
    r"cpu: (?P<vendeur>\S+) \d+/\d+/\d+ \"(?P<marque>[^\"]*)\", pa (?P<pa>\d+) va (?P<va>\d+),"
    r" fpu (?P<fpu>\d+) o,(?P<drapeaux>[a-z0-9 ]*)"
)
MOTIF_TSC = re.compile(r"time: TSC annoncé (\d+) Hz, mesuré (\d+) Hz")
MOTIF_HORLOGE = re.compile(r"time: horloge (\S+) à (\d+) Hz")
MOTIF_BOOT_OK = re.compile(r"^\[\s*(\d+\.\d+)\] I VelumOS boot ok", re.MULTILINE)
MOTIF_PMM = re.compile(r"pmm: total=(\d+) libre=(\d+) meta=(\d+) reserve=(\d+) Kio")
MOTIF_CPUS_ACPI = re.compile(r"acpi: (\d+) CPU,")
MOTIF_TABLE_HPET = re.compile(r"acpi: HPET @0x[0-9a-f]+ long")
MOTIF_AVERT_QEMU = re.compile(
    r"(?:TCG|host) doesn't support requested feature: CPUID\[[^\]]*\]\.[A-Z]+\.([\w.-]+)"
)


def lire_observations(texte_serie):
    obs = {}
    cpu = MOTIF_CPU.search(texte_serie)
    if cpu:
        obs["drapeaux"] = cpu.group("drapeaux").split()
        obs["marque"] = cpu.group("marque")
        obs["fpu"] = int(cpu.group("fpu"))
    tsc = MOTIF_TSC.search(texte_serie)
    if tsc:
        obs["tsc_annonce"], obs["tsc_mesure"] = int(tsc.group(1)), int(tsc.group(2))
    horloge = MOTIF_HORLOGE.search(texte_serie)
    if horloge:
        obs["horloge"], obs["horloge_hz"] = horloge.group(1), int(horloge.group(2))
    boot = MOTIF_BOOT_OK.search(texte_serie)
    if boot:
        obs["boot_ok_s"] = float(boot.group(1))
    pmm = MOTIF_PMM.search(texte_serie)
    if pmm:
        obs["pmm_total_kio"], obs["pmm_libre_kio"] = int(pmm.group(1)), int(pmm.group(2))
        obs["pmm_reserve_kio"] = int(pmm.group(4))
    obs["table_hpet"] = MOTIF_TABLE_HPET.search(texte_serie) is not None
    cpus = MOTIF_CPUS_ACPI.search(texte_serie)
    if cpus:
        obs["cpus_acpi"] = int(cpus.group(1))
    obs["avertissements_noyau"] = re.findall(r"^\[[^\]]*\] W (.*)$", texte_serie, re.MULTILINE)
    return obs


def verifier_protections(obs, config, attendues, texte_erreurs_qemu):
    ecarts = []
    presentes = set(obs.get("drapeaux", ()))
    filtrees = {normaliser(m.lower()) for m in MOTIF_AVERT_QEMU.findall(texte_erreurs_qemu)}
    for nom in PROTECTIONS:
        if nom not in attendues:
            continue
        if nom in presentes and not attendues[nom]:
            ecarts.append(f"{nom} annoncé alors que QEMU ne le fournit pas")
        if nom not in presentes and attendues[nom] and nom not in filtrees:
            ecarts.append(f"{nom} absent alors que QEMU le fournit")
    for nom in desactives_explicitement(config.cpu):
        if nom in presentes:
            ecarts.append(f"{nom} désactivé par -{nom} mais annoncé")
    return ecarts


def desactives_explicitement(spec):
    return [o[1:] for o in spec.split(",")[1:] if o.startswith("-") and o[1:] in PROTECTIONS]


def verifier_memoire(obs, config, texte_serie=""):
    total = obs.get("pmm_total_kio")
    if total is None:
        return ["ligne pmm absente"]
    ecarts = []
    mem_kio = taille_en_kio(config.memoire)
    plancher = mem_kio - MARGE_NOYAU_KIO - mem_kio // 32
    if total <= 0 or total > mem_kio:
        ecarts.append(f"pmm total {total} Kio hors de ]0, {mem_kio}] Kio")
    if total < plancher:
        ecarts.append(f"pmm total {total} Kio : moins que {plancher} Kio vus sur {mem_kio} Kio")
    if obs.get("pmm_libre_kio") != total:
        ecarts.append(f"libre {obs.get('pmm_libre_kio')} != total {total} avant tout usage")
    if mem_kio > SEUIL_4GIO_KIO and not plages_hautes(texte_serie):
        ecarts.append("aucune plage libre au-dessus de 4 Gio")
    return ecarts


def plages_hautes(texte_serie):
    debuts = re.findall(r"pmm: plage 0x([0-9a-f]+)-0x[0-9a-f]+ libre", texte_serie)
    return [d for d in debuts if int(d, 16) >= 1 << 32]


def taille_en_kio(texte):
    nombre, unite = int(texte[:-1]), texte[-1]
    return nombre * {"K": 1, "M": 1024, "G": 1024 * 1024, "T": 1024**3}[unite]


def verifier_horloges(obs):
    ecarts = []
    if "tsc_mesure" not in obs:
        return ["ligne time: TSC absente"]
    if obs["tsc_mesure"] <= 0:
        ecarts.append("fréquence TSC mesurée nulle")
    if "horloge" not in obs:
        ecarts.append("ligne time: horloge absente")
    return ecarts


def verifier_smp(obs, config):
    vus = obs.get("cpus_acpi")
    if vus is not None and vus != config.smp:
        return [f"ACPI annonce {vus} CPU pour -smp {config.smp}"]
    return []


def hpet_demande(machine):
    options = machine.split(",")[1:]
    return "hpet=off" not in options


def verifier_hpet(obs, config):
    ecarts = []
    if "table_hpet" not in obs:
        return ["observation HPET absente"]
    if hpet_demande(config.machine):
        if not obs["table_hpet"]:
            ecarts.append("table ACPI HPET absente alors que hpet est actif")
        return ecarts
    if obs["table_hpet"]:
        ecarts.append("table ACPI HPET présente avec hpet=off")
    if obs.get("horloge") == "HPET":
        ecarts.append("horloge HPET retenue avec hpet=off")
    return ecarts
