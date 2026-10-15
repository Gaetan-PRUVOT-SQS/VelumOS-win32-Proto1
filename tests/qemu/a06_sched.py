NAME = "ordonnanceur : autotests a06 sur un CPU"
CMDLINE = "selftest exit"
SMP = 1
FIRMWARES = ("bios", "uefi")

AUTOTESTS = (
    "équité",
    "préemption",
    "ping-pong",
    "sommeil",
    "mutex",
    "join",
    "création en masse",
    "échecs d'allocation",
)


def run(vm):
    vm.attendre(r"sched: prêt, quantum 15000 us", delai=60)
    for nom in AUTOTESTS:
        vm.attendre(rf"sched: autotest '{nom}' ok", delai=120)
    vm.attendre(r"SELFTESTS PASS \d+", delai=120)
