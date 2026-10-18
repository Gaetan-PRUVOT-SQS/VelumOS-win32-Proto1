.PHONY: hdrcheck matrice matrice-check scan test-tools test-qemu test ci run run-uefi lot lot-compile

hdrcheck:
	@for h in $$(find include -name '*.h' | sort); do \
		printf '#include "%s"\n' $${h#include/} | \
		$(CC) $(KCFLAGS) -x c -fsyntax-only - || exit 1; \
	done; echo 'en-têtes autonomes : OK'

LOTOBJ := $(patsubst %,$(O)/obj/%.o,$(KSRC_$(LOT))) $(ULOT_OBJ)

lot-compile: $(LOTOBJ)

lot: lot-compile host-$(LOT)
	@echo 'lot $(LOT) : compilation et tests hôte OK'

test-qemu: $(O)/kernel.elf $(O)/rootfs.stamp
	python3 tools/run_scenarios.py --kernel $(O)/kernel.elf --root $(O)/root \
		--out $(B)/qemu --resolution $(MODE)

matrice:
	python3 tools/matrice_couverture.py

matrice-check:
	python3 tools/matrice_couverture.py --check

scan:
	python3 tools/scan_securite.py

test-tools:
	python3 -m pytest -q tests/tools

test: test-tools test-host test-qemu

KVM := $(shell test -w /dev/kvm && echo -enable-kvm -cpu host)
OVMF_CODE ?= $(firstword $(wildcard /usr/share/edk2/ovmf/OVMF_CODE.fd \
	/usr/share/OVMF/OVMF_CODE_4M.fd /usr/share/OVMF/OVMF_CODE.fd \
	/usr/share/edk2/x64/OVMF_CODE.4m.fd))
OVMF_VARS ?= $(firstword $(wildcard /usr/share/edk2/ovmf/OVMF_VARS.fd \
	/usr/share/OVMF/OVMF_VARS_4M.fd /usr/share/OVMF/OVMF_VARS.fd \
	/usr/share/edk2/x64/OVMF_VARS.4m.fd))
UEFI_FLAGS := -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
	-drive if=pflash,format=raw,file=$(O)/ovmf_vars.fd

run: $(O)/disk.img
	qemu-system-x86_64 -machine q35 -m 256M $(KVM) -drive file=$(O)/disk.img,if=none,id=sys,format=raw,snapshot=on \
		-device virtio-blk-pci,drive=sys,disable-legacy=on \
		-serial stdio -vga std -device isa-debug-exit,iobase=0xf4,iosize=0x04

run-uefi: $(O)/disk.img
	cp $(OVMF_VARS) $(O)/ovmf_vars.fd
	qemu-system-x86_64 -machine q35 -m 256M $(KVM) $(UEFI_FLAGS) \
		-drive file=$(O)/disk.img,if=none,id=sys,format=raw,snapshot=on \
		-device virtio-blk-pci,drive=sys,disable-legacy=on \
		-serial stdio -vga std -device isa-debug-exit,iobase=0xf4,iosize=0x04

ci:
	tools/ci.sh
