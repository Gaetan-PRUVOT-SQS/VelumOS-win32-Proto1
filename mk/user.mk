UCC := $(CROSS)gcc
UAR := $(CROSS)ar
UCFLAGS := -std=gnu11 -ffreestanding -fPIE -fno-plt -fno-strict-aliasing \
	-fwrapv -fno-common -fno-omit-frame-pointer -fstack-protector-strong \
	-mstack-protector-guard=global -ftrivial-auto-var-init=zero \
	-ffunction-sections -fdata-sections -Wall -Wextra -Werror -O2 -g \
	-ffile-prefix-map=$(CURDIR)=. -Iinclude -Iuser/include -DVELUM_USER
UASFLAGS := -ffreestanding -fPIE -ffile-prefix-map=$(CURDIR)=. -Iinclude
ULDFLAGS := -nostdlib -pie -Wl,--no-dynamic-linker -Wl,-z,text \
	-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,--build-id=sha1 \
	-Wl,--gc-sections -Wl,-z,max-page-size=0x1000

$(O)/uobj/%.c.o: %.c
	@mkdir -p $(dir $@)
	@printf '  UCC  %s\n' $<
	$(Q)$(UCC) $(UCFLAGS) -MMD -MP -c $< -o $@

$(O)/uobj/%.S.o: %.S
	@mkdir -p $(dir $@)
	@printf '  UAS  %s\n' $<
	$(Q)$(UCC) $(UASFLAGS) -MMD -MP -c $< -o $@

define ulib_rule
$(O)/ulib/lib$(1).a: $$(patsubst %,$(O)/uobj/%.o,$$(ULIB_$(1)_SRC))
	@mkdir -p $$(dir $$@)
	rm -f $$@
	$(UAR) rcsD $$@ $$^
endef

define uapp_rule
$(O)/root/system/bin/$(1): $$(patsubst %,$(O)/uobj/%.o,$$(UAPP_$(1)_SRC)) \
	$$(foreach l,$$(UAPP_$(1)_LIBS),$(O)/ulib/lib$$(l).a) user/app.ld
	@mkdir -p $$(dir $$@)
	$(UCC) $$(ULDFLAGS) -T user/app.ld -o $$@ \
		$$(patsubst %,$(O)/uobj/%.o,$$(UAPP_$(1)_SRC)) \
		-Wl,--start-group $$(foreach l,$$(UAPP_$(1)_LIBS),$(O)/ulib/lib$$(l).a) \
		-Wl,--end-group -lgcc
ROOTFS_FILES += $(O)/root/system/bin/$(1)
endef

$(foreach l,$(ULIBS),$(if $(filter $(ULIB_$(l)_LOT),$(SEL)),$(eval $(call ulib_rule,$(l)))))
$(foreach a,$(UAPPS),$(if $(filter $(UAPP_$(a)_LOT),$(SEL)),$(eval $(call uapp_rule,$(a)))))

USRC := $(foreach l,$(ULIBS),$(ULIB_$(l)_SRC)) \
	$(foreach a,$(UAPPS),$(UAPP_$(a)_SRC))
UDEP := $(patsubst %,$(O)/uobj/%.d,$(USRC))
-include $(UDEP)

ULOT_OBJ := $(foreach l,$(ULIBS),$(if $(filter $(ULIB_$(l)_LOT),$(LOT)),\
	$(patsubst %,$(O)/uobj/%.o,$(ULIB_$(l)_SRC))))
ULOT_OBJ += $(foreach a,$(UAPPS),$(if $(filter $(UAPP_$(a)_LOT),$(LOT)),\
	$(patsubst %,$(O)/uobj/%.o,$(UAPP_$(a)_SRC))))

.PHONY: user
user: $(ROOTFS_FILES)

$(O)/rootfs.stamp: $(ROOTFS_FILES)
	@mkdir -p $(O)/root/system
	@touch $@
