import dataclasses
import os
import re
import shutil
import socket
import subprocess
import time
from pathlib import Path

OVMF_CANDIDATS = (
    ("/usr/share/edk2/ovmf/OVMF_CODE.fd", "/usr/share/edk2/ovmf/OVMF_VARS.fd"),
    ("/usr/share/OVMF/OVMF_CODE_4M.fd", "/usr/share/OVMF/OVMF_VARS_4M.fd"),
    ("/usr/share/OVMF/OVMF_CODE.fd", "/usr/share/OVMF/OVMF_VARS.fd"),
    ("/usr/share/edk2/x64/OVMF_CODE.4m.fd", "/usr/share/edk2/x64/OVMF_VARS.4m.fd"),
)


def trouver_ovmf():
    code, variables = os.environ.get("OVMF_CODE"), os.environ.get("OVMF_VARS")
    if code and variables:
        return code, variables
    for candidat in OVMF_CANDIDATS:
        if all(os.path.exists(c) for c in candidat):
            return candidat
    raise FileNotFoundError("OVMF introuvable : définir OVMF_CODE et OVMF_VARS")


OVMF_CODE, OVMF_VARS = trouver_ovmf()


class VMError(Exception):
    pass


class ConfigurationIncompatible(VMError):
    pass


ACCELERATEURS = ("tcg", "kvm")
ICOUNT_DEFAUT = "shift=0,sleep=off"
DEBUG_QEMU = "int,cpu_reset,guest_errors"
MOTIF_MEMOIRE = re.compile(r"[1-9][0-9]*[KMGT]?")
MOTIF_MACHINE = re.compile(r"[A-Za-z0-9_.-]+(,[A-Za-z0-9_.-]+=[A-Za-z0-9_.-]+)*")
MOTIF_CPU = re.compile(r"[A-Za-z0-9_.-]+(,([+-][A-Za-z0-9_.-]+|[A-Za-z0-9_.-]+=[A-Za-z0-9_.-]+))*")
MOTIF_ICOUNT = re.compile(r"[a-z]+=[A-Za-z0-9]+(,[a-z]+=[A-Za-z0-9]+)*")


def _valider(nom, valeur, motif):
    if valeur is not None and motif.fullmatch(valeur) is None:
        raise VMError(f"{nom} : valeur refusée {valeur!r}")
    return valeur


@dataclasses.dataclass(frozen=True)
class Reglages:
    accel: str | None = None
    cpu: str | None = None
    machine: str | None = None
    smp: int | None = None
    memoire: str | None = None
    debug: bool = False
    icount: str | None = None

    def __post_init__(self):
        if self.accel is not None and self.accel not in ACCELERATEURS:
            raise VMError(f"accélérateur inconnu {self.accel!r} (attendu : tcg ou kvm)")
        if self.smp is not None and self.smp < 1:
            raise VMError(f"smp : valeur refusée {self.smp!r}")
        _valider("cpu", self.cpu, MOTIF_CPU)
        _valider("machine", self.machine, MOTIF_MACHINE)
        _valider("mémoire", self.memoire, MOTIF_MEMOIRE)
        _valider("icount", self.icount, MOTIF_ICOUNT)

    @classmethod
    def depuis_env(cls, env=None):
        env = os.environ if env is None else env
        smp = env.get("VTEST_SMP") or None
        icount = env.get("VTEST_ICOUNT") or None
        if icount in ("0", "off"):
            icount = None
        elif icount in ("1", "on"):
            icount = ICOUNT_DEFAUT
        return cls(
            accel=env.get("VTEST_ACCEL") or None,
            cpu=env.get("VTEST_CPU") or None,
            machine=env.get("VTEST_MACHINE") or None,
            smp=int(smp) if smp is not None else None,
            memoire=env.get("VTEST_MEM") or None,
            debug=env.get("VTEST_DEBUG", "") not in ("", "0"),
            icount=icount,
        )

    def par_dessus(self, autre):
        valeurs = {
            champ.name: getattr(autre, champ.name)
            for champ in dataclasses.fields(self)
            if getattr(autre, champ.name) not in (None, False)
        }
        return dataclasses.replace(self, **valeurs)


class Image:
    def __init__(self, largeur, hauteur, donnees):
        self.largeur = largeur
        self.hauteur = hauteur
        self.donnees = donnees

    @classmethod
    def lire_ppm(cls, chemin):
        brut = Path(chemin).read_bytes()
        champs = re.match(rb"P6\s+(\d+)\s+(\d+)\s+255\s", brut)
        if champs is None:
            raise VMError(f"ppm illisible : {chemin}")
        largeur, hauteur = int(champs.group(1)), int(champs.group(2))
        return cls(largeur, hauteur, brut[champs.end() :])

    def pixel(self, x, y):
        debut = (y * self.largeur + x) * 3
        return tuple(self.donnees[debut : debut + 3])

    def compter(self, rect, couleur, tolerance=0):
        x0, y0, largeur, hauteur = rect
        total = 0
        for y in range(y0, y0 + hauteur):
            for x in range(x0, x0 + largeur):
                reel = self.pixel(x, y)
                if all(abs(a - b) <= tolerance for a, b in zip(reel, couleur, strict=True)):
                    total += 1
        return total

    def differences(self, autre, tolerance=0):
        if (self.largeur, self.hauteur) != (autre.largeur, autre.hauteur):
            return self.largeur * self.hauteur
        total = 0
        for i in range(0, len(self.donnees), 3):
            a, b = self.donnees[i : i + 3], autre.donnees[i : i + 3]
            if any(abs(p - q) > tolerance for p, q in zip(a, b, strict=True)):
                total += 1
        return total

    def enregistrer_png(self, chemin):
        from PIL import Image as PilImage

        PilImage.frombytes("RGB", (self.largeur, self.hauteur), bytes(self.donnees)).save(
            chemin, optimize=True
        )


class VM:
    def __init__(
        self,
        disque,
        dossier,
        nom="vm",
        uefi=False,
        memoire=None,
        smp=None,
        extra=(),
        reglages=None,
    ):
        self.reglages = Reglages.depuis_env() if reglages is None else reglages
        self.disque = Path(disque)
        self.dossier = Path(dossier)
        self.nom = nom
        self.uefi = uefi
        self.memoire = memoire or self.reglages.memoire or "256M"
        self.smp = smp or self.reglages.smp or 1
        self.extra = list(extra)
        self.processus = None
        self.avertissements = []
        self.commande_lancee = []
        self.journal = self.dossier / f"{nom}.serie.log"
        self.erreurs = self.dossier / f"{nom}.stderr.log"
        self.journal_debug = self.dossier / f"{nom}.qemu-debug.log"
        self.socket_moniteur = self.dossier / f"{nom}.mon"
        self.capture_n = 0

    def accelerateur(self):
        demande = self.reglages.accel
        kvm_disponible = os.access("/dev/kvm", os.W_OK)
        if demande == "kvm" and not kvm_disponible:
            raise ConfigurationIncompatible("KVM demandé mais /dev/kvm inaccessible")
        if demande is None and (self.reglages.debug or self.reglages.icount):
            return "tcg", False
        if demande is None:
            sans_kvm = os.environ.get("VTEST_SANS_KVM")
            return ("kvm" if kvm_disponible and not sans_kvm else "tcg"), False
        return demande, True

    def options_accelerateur(self):
        accel, explicite = self.accelerateur()
        cpu = self.reglages.cpu
        if accel == "kvm":
            options = ["-enable-kvm", "-cpu", cpu or "host"]
        else:
            if cpu and cpu.split(",")[0] == "host":
                raise ConfigurationIncompatible(f"-cpu {cpu} exige KVM")
            options = (["-accel", "tcg"] if explicite else []) + ["-cpu", cpu or "max"]
        if self.reglages.icount:
            if accel != "tcg":
                raise ConfigurationIncompatible("-icount exige TCG")
            options += ["-icount", self.reglages.icount]
        if self.reglages.debug:
            if accel == "tcg":
                options += ["-d", DEBUG_QEMU, "-D", str(self.journal_debug)]
            else:
                self.avertissements.append("VTEST_DEBUG ignoré : -d exige TCG")
        return options

    def commande(self):
        self.avertissements.clear()
        base = [
            "qemu-system-x86_64",
            "-machine",
            self.reglages.machine or "q35",
            "-m",
            self.memoire,
            "-smp",
            str(self.smp),
            *self.options_accelerateur(),
            "-display",
            "none",
            "-vga",
            "std",
            "-no-reboot",
            "-drive",
            f"file={self.disque},format=raw,snapshot=on",
            "-serial",
            f"file:{self.journal}",
            "-monitor",
            f"unix:{self.socket_moniteur},server,nowait",
            "-device",
            "isa-debug-exit,iobase=0xf4,iosize=0x04",
        ]
        if self.uefi:
            vars_copie = self.dossier / f"{self.nom}.vars.fd"
            shutil.copyfile(OVMF_VARS, vars_copie)
            base += [
                "-drive",
                f"if=pflash,format=raw,readonly=on,file={OVMF_CODE}",
                "-drive",
                f"if=pflash,format=raw,file={vars_copie}",
            ]
        return base + self.extra

    def demarrer(self):
        self.dossier.mkdir(parents=True, exist_ok=True)
        self.journal.write_text("")
        self.socket_moniteur.unlink(missing_ok=True)
        commande = self.commande()
        self.commande_lancee = commande
        with self.erreurs.open("w") as erreurs:
            self.processus = subprocess.Popen(commande, stdout=subprocess.DEVNULL, stderr=erreurs)
        debut = time.monotonic()
        while not self.socket_moniteur.exists():
            if self.processus.poll() is not None:
                raise VMError(
                    f"QEMU a quitté au lancement (code {self.processus.returncode}) :"
                    f" {self.erreurs_qemu()[-1000:]}"
                )
            if time.monotonic() - debut > 10:
                raise VMError("moniteur QEMU absent")
            time.sleep(0.05)
        return self

    def erreurs_qemu(self):
        try:
            return self.erreurs.read_text(errors="replace")
        except OSError:
            return ""

    def serie(self):
        return self.journal.read_text(errors="replace")

    def attendre(self, motif, delai=30):
        regex = re.compile(motif)
        fin = time.monotonic() + delai
        while time.monotonic() < fin:
            texte = self.serie()
            trouve = regex.search(texte)
            if trouve:
                return trouve
            if "PANIC:" in texte and "PANIC" not in motif:
                raise VMError(f"panique pendant l'attente de {motif!r}\n{texte[-2000:]}")
            if self.processus.poll() is not None:
                raise VMError(f"QEMU arrêté (code {self.processus.returncode})\n{texte[-2000:]}")
            time.sleep(0.05)
        raise VMError(f"délai dépassé pour {motif!r}\n{self.serie()[-2000:]}")

    def moniteur(self, commande, delai=5):
        with socket.socket(socket.AF_UNIX) as sock:
            sock.settimeout(delai)
            sock.connect(str(self.socket_moniteur))
            self._lire_invite(sock)
            sock.sendall(commande.encode() + b"\n")
            return self._lire_invite(sock)

    @staticmethod
    def _lire_invite(sock):
        tampon = b""
        while not tampon.endswith(b"(qemu) "):
            morceau = sock.recv(4096)
            if not morceau:
                break
            tampon += morceau
        return tampon.decode(errors="replace")

    def capture(self, nom=None):
        self.capture_n += 1
        chemin = self.dossier / f"{self.nom}.{nom or str(self.capture_n) + '.ppm'}"
        self.moniteur(f"screendump {chemin}")
        fin = time.monotonic() + 5
        while not chemin.exists() and time.monotonic() < fin:
            time.sleep(0.05)
        return Image.lire_ppm(chemin)

    def capture_quand(self, condition, delai=5.0, pause=0.05, nom=None):
        fin = time.monotonic() + delai
        while True:
            image = self.capture(nom)
            if condition(image) or time.monotonic() >= fin:
                return image
            time.sleep(pause)

    def capture_stable(self, delai=5.0, pause=0.1, tolerance=200, nom=None):
        fin = time.monotonic() + delai
        precedente = self.capture(nom)
        while time.monotonic() < fin:
            time.sleep(pause)
            courante = self.capture(nom)
            if courante.differences(precedente) <= tolerance:
                return courante
            precedente = courante
        return precedente

    def touche(self, nom):
        self.moniteur(f"sendkey {nom}")

    def souris_deplacer(self, dx, dy):
        self.moniteur(f"mouse_move {dx} {dy}")

    def souris_bouton(self, masque):
        self.moniteur(f"mouse_button {masque}")

    def arreter(self):
        if self.processus and self.processus.poll() is None:
            self.processus.kill()
        if self.processus:
            self.processus.wait()

    def __enter__(self):
        return self.demarrer()

    def __exit__(self, *exc):
        self.arreter()
