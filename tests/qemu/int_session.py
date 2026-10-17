import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from int_captures import HAUTEUR, LARGEUR, verifier_ecran  # noqa: E402

NAME = "intégration : session jusqu'à l'écran de connexion (maquette 01)"
CMDLINE = "quiet"
MEMOIRE = "256M"


def run(vm):
    vm.attendre(r"init: démarrage de la session", delai=60)
    vm.attendre(r"logon: écran de connexion prêt", delai=120)
    time.sleep(1)
    image = vm.capture_stable(nom="session.ppm")
    image.enregistrer_png(vm.dossier / f"{vm.nom}.session.png")
    assert (image.largeur, image.hauteur) == (LARGEUR, HAUTEUR)
    texte = vm.serie()
    assert "PANIC:" not in texte
    ecarts = []
    verifier_ecran(ecarts, "connexion", image)
    assert not ecarts, "écarts à la maquette 01 :\n  " + "\n  ".join(ecarts)
