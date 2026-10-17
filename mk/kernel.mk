SEL := $(or $(ONLY),$(LOTS))
KSRC_ALL := $(foreach l,$(SEL),$(KSRC_$(l)))
KOBJ := $(patsubst %,$(O)/obj/%.o,$(KSRC_ALL))
KDEP := $(KOBJ:.o=.d)

$(O)/obj/%.c.o: %.c
	@mkdir -p $(dir $@)
	@printf '  CC   %s\n' $<
	$(Q)$(CC) $(KCFLAGS) -MMD -MP -c $< -o $@

$(O)/obj/arch/x86_64/cpu/trap_report.c.o: KCFLAGS += -Wframe-larger-than=4096

$(O)/obj/%.S.o: %.S
	@mkdir -p $(dir $@)
	@printf '  AS   %s\n' $<
	$(Q)$(CC) $(KASFLAGS) -MMD -MP -c $< -o $@

$(O)/kernel.elf: $(KOBJ) link/kernel.ld
	@printf '  LD   %s\n' $@
	$(Q)$(LD) -nostdlib -static -T link/kernel.ld -z max-page-size=0x1000 \
		-z noexecstack -Wl,--build-id=sha1 -Wl,--gc-sections $(KOBJ) -lgcc -o $@

$(O)/disk.cfg: FORCE
	@mkdir -p $(O)
	@printf '%s\n' '$(CMDLINE)|$(MODE)' | cmp -s - $@ || printf '%s\n' '$(CMDLINE)|$(MODE)' > $@

FORCE:

$(O)/disk.img: $(O)/kernel.elf $(O)/rootfs.stamp $(O)/disk.cfg tools/mkimage.py $(LIMINE_DIR)/limine
	@printf '  IMG  %s\n' $@
	$(Q)python3 tools/mkimage.py --noyau $(O)/kernel.elf --sortie $@ \
		--initrd $(O)/root --resolution $(MODE) --cmdline "$(CMDLINE)"

.PHONY: kernel FORCE
kernel: $(O)/kernel.elf

-include $(KDEP)
