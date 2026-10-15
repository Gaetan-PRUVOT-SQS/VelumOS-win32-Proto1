NAME = "intégration : préemption et mort d'un processus qui boucle en anneau 3"
CMDLINE = "init=/system/bin/spin"


def run(vm):
    vm.attendre(r"SPIN start", delai=60)
    vm.attendre(r"SPIN PASS", delai=30)
    vm.attendre(r"KILL PASS", delai=30)
