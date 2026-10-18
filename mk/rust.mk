comma := ,
empty :=
space := $(empty) $(empty)
RUST_FEATURES ?=
RUST_FEAT_LIST := $(subst $(comma), ,$(RUST_FEATURES))
RUST_DROP :=
RUST_HOST_LINK :=
RUST_HOST_EXTRA :=
RUST_USER_EXTRA :=
RUST_KERN_LINK :=
RUST_USER_LINK :=

ifneq ($(strip $(RUST_FEATURES)),)
RUST_HOST_DIR := $(B)/cargo-host
RUST_KERN_DIR := $(B)/cargo-kern
RUST_USER_DIR := $(B)/cargo-user
RUST_HOST_LINK := $(RUST_HOST_DIR)/x86_64-unknown-linux-gnu/release/libvelum_link.a
RUST_KERN_FEATS := $(subst $(space),$(comma),$(filter libk crypto gfx font,$(RUST_FEAT_LIST)))
RUST_USER_FEATS := $(subst $(space),$(comma),$(filter libk crypto gfx font zip rsa axml dex dexcode notif settings pm apk wm,$(RUST_FEAT_LIST)))
ifneq ($(strip $(RUST_KERN_FEATS)),)
RUST_KERN_LINK := $(RUST_KERN_DIR)/x86_64-unknown-none/release/libvelum_link.a
endif
ifneq ($(strip $(RUST_USER_FEATS)),)
RUST_USER_LINK := $(RUST_USER_DIR)/x86_64-unknown-none/release/libvelum_link.a
endif

ifneq ($(filter libk,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c lib/libk/mem.S
endif
ifneq ($(filter crypto,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/crypto/%
endif
ifneq ($(filter zip,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/zip/%
endif
ifneq ($(filter gfx,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/gfx/%
endif
ifneq ($(filter font,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/font/%
endif
ifneq ($(filter rsa,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/rsa/%
endif
ifneq ($(filter axml,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/axml/%
endif
ifneq ($(filter dex,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/dex/%
endif
ifneq ($(filter dexcode,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/dexcode/%
endif
ifneq ($(filter notif,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/notif/%
endif
ifneq ($(filter settings,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/settings/%
endif
ifneq ($(filter pm,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/pm/%
endif
ifneq ($(filter apk,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/apk/%
RUST_HOST_EXTRA += $(B)/apk_host_alloc.o
endif
ifneq ($(filter wm,$(RUST_FEAT_LIST)),)
RUST_DROP += lib/wm/%
RUST_HOST_EXTRA += $(B)/wm_luna.o
RUST_USER_EXTRA += $(B)/wm_luna_user.o
endif

RUST_HOST_FLAGS := --cfg velum_host -C panic=abort
RUST_KERN_FLAGS := --cfg velum_kernel -C panic=abort -C code-model=kernel \
	-C relocation-model=static -C no-redzone=yes \
	-C target-feature=-sse,-sse2
RUST_USER_FLAGS := --cfg velum_user -C panic=abort -C code-model=small \
	-C relocation-model=pic

$(B)/apk_host_alloc.o: rust/support/apk_host_alloc.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTFLAGS) -c $< -o $@

$(B)/wm_luna.o: rust/support/wm_luna.c
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTFLAGS) -c $< -o $@

$(B)/wm_luna_user.o: rust/support/wm_luna.c
	@mkdir -p $(dir $@)
	$(UCC) $(UCFLAGS) -c $< -o $@

$(RUST_HOST_LINK): FORCE
	@mkdir -p $(RUST_HOST_DIR)
	cd rust && CARGO_TARGET_DIR=$(abspath $(RUST_HOST_DIR)) \
		RUSTFLAGS='$(RUST_HOST_FLAGS)' \
		cargo rustc -p velum-link --release --crate-type staticlib --offline \
		--features handler,$(RUST_FEATURES) --target x86_64-unknown-linux-gnu

ifneq ($(strip $(RUST_KERN_FEATS)),)
$(RUST_KERN_LINK): FORCE
	@mkdir -p $(RUST_KERN_DIR)
	cd rust && CARGO_TARGET_DIR=$(abspath $(RUST_KERN_DIR)) \
		RUSTFLAGS='$(RUST_KERN_FLAGS)' \
		cargo rustc -p velum-link --release --crate-type staticlib --offline \
		--features handler,$(RUST_KERN_FEATS) --target x86_64-unknown-none
endif

ifneq ($(strip $(RUST_USER_FEATS)),)
$(RUST_USER_LINK): FORCE
	@mkdir -p $(RUST_USER_DIR)
	cd rust && CARGO_TARGET_DIR=$(abspath $(RUST_USER_DIR)) \
		RUSTFLAGS='$(RUST_USER_FLAGS)' \
		cargo rustc -p velum-link --release --crate-type staticlib --offline \
		--features handler,$(RUST_USER_FEATS) --target x86_64-unknown-none
endif

A20_RLIBS := $(filter-out $(RUST_DROP),$(A20_RLIBS))
A20_ROBJ := $(patsubst %.c,$(B)/host/a20r/obj/%.o,$(A20_RFAKES) $(A20_RAPP) \
	$(A20_RLIBS))

$(B)/host/a20r/%: $(A20_RDIR)/%.c $(A20_ROBJ) $(RUST_HOST_LINK) $(RUST_HOST_EXTRA)
	$(HOSTCC) $(A20_RFLAGS) $< $(A20_ROBJ) $(RUST_HOST_EXTRA) \
		-lc -lgcc $(RUST_HOST_LINK) -o $@

ifneq ($(filter font,$(RUST_FEAT_LIST)),)
$(A16_XCHECK): tests/host/a16/crosscheck_utf8.c $(RUST_HOST_LINK)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTFLAGS) -Ilib/font -Itests/host/a16 \
		tests/host/a16/crosscheck_utf8.c tests/host/a16/fake_gfx.c \
		-lc -lgcc $(RUST_HOST_LINK) -o $@

$(A16_INTEG): tests/host/a16/integ_gfx.c tests/host/a16/fake_buf.c \
	tests/host/a16/fake_font.c tests/host/a16/fake_paint.c tests/host/harness.c \
	tests/host/harness_eq.c $(RUST_HOST_LINK)
	@mkdir -p $(dir $@)
	$(HOSTCC) $(HOSTFLAGS) -Ilib/font -Ilib/gfx -Itests/host/a16 \
		tests/host/a16/integ_gfx.c tests/host/a16/fake_buf.c \
		tests/host/a16/fake_font.c tests/host/a16/fake_paint.c \
		tests/host/harness.c tests/host/harness_eq.c \
		$(filter-out $(RUST_DROP),$(KSRC_a16) $(sort $(wildcard lib/gfx/*.c))) \
		lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c \
		-lc -lgcc $(RUST_HOST_LINK) -o $@
endif
endif
