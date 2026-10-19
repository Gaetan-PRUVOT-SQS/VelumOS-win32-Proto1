SHELL := /bin/sh
.SUFFIXES:
export LC_ALL := C
export SOURCE_DATE_EPOCH := 1760000000

B ?= build/main
PROFIL ?= debug
O := $(B)/$(PROFIL)
CMDLINE ?= quiet
MODE ?= 1024x768
LOT ?= skel
V ?= 0
Q := @
ifeq ($(V),1)
Q :=
endif
HOSTCC ?= gcc
LIMINE_VERSION := 12.9.1
LIMINE_SHA256 := ce972a05e9d1973dc9b725f9130bd15af01add0eb3a7f90c118ac5cfe21c17b8
LIMINE_DIR ?= build/limine
export LIMINE_DIR

CROSS ?= x86_64-elf-
CC := $(CROSS)gcc
LD := $(CROSS)gcc
AR := $(CROSS)ar
INC := -Iinclude -Ithird_party/limine -Iarch/x86_64/limine

KCFLAGS_COMMON := -std=gnu11 -ffreestanding -fno-pic -fno-pie -mcmodel=kernel \
	-mno-red-zone -mgeneral-regs-only -fno-omit-frame-pointer \
	-fno-strict-aliasing -fwrapv -fno-common -fno-delete-null-pointer-checks \
	-fstack-protector-strong -mstack-protector-guard=global \
	-ftrivial-auto-var-init=zero -fzero-call-used-regs=used-gpr \
	-ffunction-sections -fdata-sections -Wall -Wextra -Werror \
	-Wvla -Wframe-larger-than=2048 -fstack-clash-protection \
	-ffile-prefix-map=$(CURDIR)=. -DVELUM_KERNEL
KCFLAGS_debug := -O1 -g -fsanitize=undefined -fsanitize-undefined-trap-on-error \
	-DVELUM_DEBUG
KCFLAGS_release := -O2 -g -DNDEBUG
KCFLAGS := $(KCFLAGS_COMMON) $(KCFLAGS_$(PROFIL)) $(INC)
KASFLAGS := -ffreestanding -mno-red-zone -ffile-prefix-map=$(CURDIR)=. $(INC)

LOTS :=
KSRC :=
UAPPS :=
ULIBS :=
ROOTFS_FILES :=

include mk/skel.mk
include $(sort $(wildcard mk/lot-*.mk))
include mk/rust.mk
include mk/kernel.mk
include mk/user.mk
include mk/host.mk
include mk/test.mk
include $(sort $(wildcard mk/qa-*.mk))

.DEFAULT_GOAL := all
.PHONY: all clean help limine

all: $(O)/kernel.elf $(O)/disk.img

clean:
	rm -rf $(B)

limine: $(LIMINE_DIR)/limine

$(LIMINE_DIR)/limine:
	mkdir -p $(LIMINE_DIR)
	curl -fsSL -o $(LIMINE_DIR)/limine-binary.tar.xz \
		https://github.com/Limine-Bootloader/Limine/releases/download/v$(LIMINE_VERSION)/limine-binary.tar.xz
	echo "$(LIMINE_SHA256)  $(LIMINE_DIR)/limine-binary.tar.xz" | sha256sum -c -
	tar -xf $(LIMINE_DIR)/limine-binary.tar.xz -C $(LIMINE_DIR) --strip-components=1
	$(HOSTCC) -O2 -std=c99 -D_FILE_OFFSET_BITS=64 $(LIMINE_DIR)/limine.c -o $@

help:
	@printf '%s\n' 'make [B=build/x] [PROFIL=debug|release]' \
	'  limine        télécharge Limine $(LIMINE_VERSION) (empreinte vérifiée) et compile son outil' \
	'  all           noyau, applis et image disque' \
	'  ONLY="skel aNN"  ne construit que ces lots (noyau, libs, applis)' \
	'  scan          contrôle statique de sécurité (fonctions dangereuses, secrets)' \
	'  matrice       recalcule les taux de couverture automatisée et manuelle (tests/MATRICE.md)' \
	'  lot LOT=aNN   compile le lot et lance ses tests hôte' \
	'  test-host     tests unitaires hôte (ASan, UBSan)' \
	'  test-qemu     scénarios QEMU (BIOS et UEFI)' \
	'  ci            compilation avec le gcc du système, tests hôte, démarrage QEMU sans KVM' \
	'  test-tools    tests des outils Python (pytest)' \
	'  test          outils, hôte, QEMU' \
	'  run           lance QEMU avec fenêtre (run-uefi en UEFI)'
