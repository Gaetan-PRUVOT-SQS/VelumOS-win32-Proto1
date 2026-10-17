MATRICE_JOBS ?= 2
MATRICE_CONFIG ?=

.PHONY: test-matrice

test-matrice: $(O)/kernel.elf $(O)/rootfs.stamp
	python3 tools/run_scenarios.py --kernel $(O)/kernel.elf --root $(O)/root \
		--out $(B)/qemu --resolution $(MODE) --matrice --jobs $(MATRICE_JOBS) \
		--rejouer-seul $(if $(MATRICE_CONFIG),--config $(MATRICE_CONFIG))
