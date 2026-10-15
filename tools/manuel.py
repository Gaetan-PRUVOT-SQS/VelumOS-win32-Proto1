#!/usr/bin/env python3
"""Pilote une machine QEMU qui reste vivante entre deux commandes, pour les cas de tests/MANUEL.md.

python3 tools/manuel.py demarrer NOM [--uefi] [--cmdline LIGNE] [--redemarrage] [--lent]
                                   [--disque IMAGE]
python3 tools/manuel.py touche NOM TOUCHE...
python3 tools/manuel.py cliquer NOM X Y [--double] [--droit]
python3 tools/manuel.py glisser NOM X0 Y0 X1 Y1
python3 tools/manuel.py capture NOM FICHIER [--zone X0 Y0 X1 Y1 [--zoom N]]
python3 tools/manuel.py rafale NOM FICHIER NOMBRE
python3 tools/manuel.py planche NOM PREFIXE
python3 tools/manuel.py serie NOM [LIGNES]
python3 tools/manuel.py arreter NOM

Tout est écrit dans build/manuel/NOM/ : journal série, captures, disque.
"""

import argparse
import hashlib
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from vtest import VM, VMError  # noqa: E402

RACINE = Path(__file__).resolve().parent.parent
SORTIE = RACINE / "build" / "manuel"
NOYAU = RACINE / "build" / "main" / "debug" / "kernel.elf"
INITRD = RACINE / "build" / "main" / "debug" / "root"
LARGEUR = 1024
PAS = 100
PAUSE = 0.12
PAUSE_TOUCHE = 0.2


class MachineManuelle(VM):
    def __init__(self, nom, uefi, redemarrage, extra, lent=False):
        super().__init__(
            SORTIE / "disques" / "nul", SORTIE / nom, nom=nom, uefi=uefi, extra=extra, smp=1
        )
        self.redemarrage = redemarrage
        self.lent = lent

    def commande(self):
        base = super().commande()
        if self.lent:
            base = " ".join(base).replace("-enable-kvm -cpu host", f"-cpu max {self.lent}").split()
        return [a for a in base if not (self.redemarrage and a == "-no-reboot")]

    def demarrer(self):
        self.dossier.mkdir(parents=True, exist_ok=True)
        self.journal.write_text("")
        self.socket_moniteur.unlink(missing_ok=True)
        pid = subprocess.Popen(
            self.commande(),
            stdout=subprocess.DEVNULL,
            stderr=(self.dossier / "qemu.err").open("w"),
            start_new_session=True,
        ).pid
        (self.dossier / "pid").write_text(str(pid))
        debut = time.monotonic()
        while not self.socket_moniteur.exists():
            if time.monotonic() - debut > 10:
                raise VMError("moniteur QEMU absent")
            time.sleep(0.05)


def image_pour(cmdline):
    cle = hashlib.sha256(cmdline.encode()).hexdigest()[:10]
    image = SORTIE / "disques" / f"disque-{cle}.img"
    image.parent.mkdir(parents=True, exist_ok=True)
    dates = [NOYAU.stat().st_mtime] + [f.stat().st_mtime for f in INITRD.rglob("*") if f.is_file()]
    if not image.exists() or image.stat().st_mtime < max(dates):
        subprocess.run(
            [
                sys.executable,
                str(RACINE / "tools" / "mkimage.py"),
                "--noyau",
                str(NOYAU),
                "--sortie",
                str(image),
                "--initrd",
                str(INITRD),
                "--resolution",
                "1024x768",
                "--cmdline",
                cmdline,
            ],
            check=True,
            capture_output=True,
        )
    return image


def machine_existante(nom):
    machine = VM(SORTIE / "disques" / "nul", SORTIE / nom, nom=nom)
    if not machine.socket_moniteur.exists():
        raise VMError(f"machine {nom} absente : lancer d'abord « demarrer {nom} »")
    return machine


def placer(machine, x, y):
    for _ in range(LARGEUR // 200 + 2):
        machine.souris_deplacer(-200, -200)
        time.sleep(PAUSE)
    deplacer(machine, x, y)


def deplacer(machine, dx, dy):
    while dx or dy:
        pas_x, pas_y = (
            min(PAS, dx) if dx > 0 else max(-PAS, dx),
            min(PAS, dy) if dy > 0 else max(-PAS, dy),
        )
        machine.souris_deplacer(pas_x, pas_y)
        time.sleep(PAUSE)
        dx, dy = dx - pas_x, dy - pas_y


def cmd_demarrer(args):
    extra = []
    for rang, fichier in enumerate(args.disque):
        extra += [
            "-drive",
            f"file={Path(fichier).resolve()},if=none,id=m{rang},format=raw",
            "-device",
            f"virtio-blk-pci,drive=m{rang},disable-legacy=on,addr=0x{0x10 + rang:x}",
        ]
    machine = MachineManuelle(args.nom, args.uefi, args.redemarrage, extra, args.lent)
    machine.disque = image_pour(args.cmdline)
    machine.memoire = args.memoire
    machine.demarrer()
    print(f"{args.nom} démarrée (disque {machine.disque.name}, série {machine.journal})")


def cmd_touche(args):
    machine = machine_existante(args.nom)
    for touche in args.touches:
        machine.touche(touche)
        time.sleep(PAUSE_TOUCHE)


def cmd_cliquer(args):
    machine = machine_existante(args.nom)
    placer(machine, args.x, args.y)
    masque = 2 if args.droit else 1
    for _ in range(2 if args.double else 1):
        machine.souris_bouton(masque)
        time.sleep(0.08)
        machine.souris_bouton(0)
        time.sleep(0.12)
    time.sleep(0.3)


def cmd_glisser(args):
    machine = machine_existante(args.nom)
    placer(machine, args.x0, args.y0)
    machine.souris_bouton(1)
    time.sleep(0.2)
    deplacer(machine, args.x1 - args.x0, args.y1 - args.y0)
    machine.souris_bouton(0)
    time.sleep(0.3)


def cmd_capture(args):
    machine = machine_existante(args.nom)
    image = machine.capture(f"{args.fichier}.ppm")
    cible = SORTIE / args.nom / f"{args.fichier}.png"
    image.enregistrer_png(cible)
    (SORTIE / args.nom / f"{args.fichier}.ppm").unlink()
    if args.zone:
        from PIL import Image

        x0, y0, x1, y1 = args.zone
        zoom = args.zoom
        morceau = Image.open(cible).crop((x0, y0, x1, y1))
        morceau.resize((morceau.width * zoom, morceau.height * zoom), Image.NEAREST).save(cible)
    print(cible)


def cmd_rafale(args):
    machine = machine_existante(args.nom)
    for i in range(args.nombre):
        image = machine.capture(f"{args.fichier}-{i:02d}.ppm")
        image.enregistrer_png(SORTIE / args.nom / f"{args.fichier}-{i:02d}.png")
        (SORTIE / args.nom / f"{args.fichier}-{i:02d}.ppm").unlink()


def cmd_planche(args):
    from PIL import Image

    vignettes = sorted((SORTIE / args.nom).glob(f"{args.prefixe}-*.png"))
    colonnes = 4
    lignes = -(-len(vignettes) // colonnes)
    planche = Image.new("RGB", (colonnes * 512, lignes * 384), "white")
    for i, chemin in enumerate(vignettes):
        planche.paste(
            Image.open(chemin).resize((512, 384)), (i % colonnes * 512, i // colonnes * 384)
        )
    cible = SORTIE / args.nom / f"{args.prefixe}-planche.png"
    planche.save(cible)
    print(cible)


def cmd_serie(args):
    machine = machine_existante(args.nom)
    print("\n".join(machine.serie().splitlines()[-args.lignes :]))


def cmd_arreter(args):
    pid_fichier = SORTIE / args.nom / "pid"
    if pid_fichier.exists():
        subprocess.run(["kill", pid_fichier.read_text().strip()], check=False)
        pid_fichier.unlink()
    (SORTIE / args.nom / f"{args.nom}.mon").unlink(missing_ok=True)


def construire():
    parseur = argparse.ArgumentParser()
    sous = parseur.add_subparsers(dest="commande", required=True)
    p = sous.add_parser("demarrer")
    p.add_argument("nom")
    p.add_argument("--uefi", action="store_true")
    p.add_argument("--cmdline", default="quiet")
    p.add_argument("--memoire", default="256M")
    p.add_argument("--redemarrage", action="store_true")
    p.add_argument("--disque", action="append", default=[], help="disque virtio-blk supplémentaire")
    p.add_argument("--lent", nargs="?", const="-accel tcg", help="sans KVM, avec ces options QEMU")
    p.set_defaults(fonction=cmd_demarrer)
    p = sous.add_parser("touche")
    p.add_argument("nom")
    p.add_argument("touches", nargs="+")
    p.set_defaults(fonction=cmd_touche)
    p = sous.add_parser("cliquer")
    p.add_argument("nom")
    p.add_argument("x", type=int)
    p.add_argument("y", type=int)
    p.add_argument("--double", action="store_true")
    p.add_argument("--droit", action="store_true")
    p.set_defaults(fonction=cmd_cliquer)
    p = sous.add_parser("glisser")
    p.add_argument("nom")
    for coordonnee in ("x0", "y0", "x1", "y1"):
        p.add_argument(coordonnee, type=int)
    p.set_defaults(fonction=cmd_glisser)
    p = sous.add_parser("capture")
    p.add_argument("nom")
    p.add_argument("fichier")
    p.add_argument("--zone", type=int, nargs=4, metavar=("X0", "Y0", "X1", "Y1"))
    p.add_argument("--zoom", type=int, default=3)
    p.set_defaults(fonction=cmd_capture)
    p = sous.add_parser("rafale")
    p.add_argument("nom")
    p.add_argument("fichier")
    p.add_argument("nombre", type=int)
    p.set_defaults(fonction=cmd_rafale)
    p = sous.add_parser("planche")
    p.add_argument("nom")
    p.add_argument("prefixe")
    p.set_defaults(fonction=cmd_planche)
    p = sous.add_parser("serie")
    p.add_argument("nom")
    p.add_argument("lignes", type=int, nargs="?", default=20)
    p.set_defaults(fonction=cmd_serie)
    p = sous.add_parser("arreter")
    p.add_argument("nom")
    p.set_defaults(fonction=cmd_arreter)
    return parseur


def main(argv):
    args = construire().parse_args(argv)
    try:
        args.fonction(args)
    except (VMError, OSError, subprocess.CalledProcessError) as erreur:
        print(f"erreur : {erreur}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
