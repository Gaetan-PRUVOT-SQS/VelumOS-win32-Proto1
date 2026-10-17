REPRO_A := $(B)/repro-a
REPRO_B := $(B)/repro-b
REPRO_SHIM := $(B)/repro-shim
REPRO_TOUCH ?= 1
REPRO_ENV_B ?=
REPRO_DIRS := arch kernel lib user drivers fs include rootfs link third_party assets tools
REPRO_SOURCES := $(wildcard $(REPRO_DIRS))

.PHONY: repro
repro:
	@rm -rf $(REPRO_A) $(REPRO_B) $(REPRO_SHIM)
	@mkdir -p $(REPRO_SHIM)
	@printf '%s\n' '#!/bin/sh' 'set -eu' \
		'ici=$$(cd -- "$$(dirname -- "$$0")" && pwd)' 'chemin=' 'IFS=:' \
		'for d in $$PATH; do [ "$$d" = "$$ici" ] || chemin="$${chemin:+$$chemin:}$$d"; done' \
		'PATH=$$chemin' 'export PATH' \
		'for a in "$$@"; do case $$a in -print0|-exec|-execdir|-ok|-delete|-fprint*) exec find "$$@";; esac; done' \
		'find "$$@" | tac' > $(REPRO_SHIM)/find
	@chmod 755 $(REPRO_SHIM)/find
	@printf '  REPRO construction A : %s (environnement normal)\n' $(REPRO_A)
	$(Q)env VELUM_REPRO=1 $(MAKE) --no-print-directory B=$(REPRO_A) PROFIL=$(PROFIL) all
	@if [ "$(REPRO_TOUCH)" = 1 ]; then \
		find $(REPRO_SOURCES) -type f -print0 | LC_ALL=C sort -z -r | xargs -0 touch --; \
	fi
	@printf '  REPRO construction B : %s (TZ, LANG, umask, find et dates des sources perturbés)\n' $(REPRO_B)
	$(Q)env VELUM_REPRO=1 TZ=Pacific/Kiritimati LANG=tr_TR.UTF-8 LANGUAGE=tr $(REPRO_ENV_B) \
		PATH="$(abspath $(REPRO_SHIM)):$$PATH" \
		sh -c 'umask 077 && exec "$$0" "$$@"' $(MAKE) --no-print-directory \
		B=$(REPRO_B) PROFIL=$(PROFIL) all
	@printf '  REPRO comparaison\n'
	$(Q)python3 tools/mkimage.py --comparer $(REPRO_A)/$(PROFIL) $(REPRO_B)/$(PROFIL)
