HOSTFLAGS := -std=gnu11 -O1 -g -fsanitize=address,undefined \
	-fno-sanitize-recover=undefined -Wall -Wextra -Werror -Iinclude -Ilib/libk \
	-Itests/host -DVELUM_HOST \
	$(foreach f,strlen strnlen strcmp strncmp strchr strrchr strstr strlcpy \
	strlcat memcmp memchr,-D$(f)=vk_$(f))
HB := $(B)/host

define host_rule
$(HB)/$(1)/$(notdir $(2:.c=)): $(2) tests/host/harness.c tests/host/harness_eq.c $$(HSRC_$(1)) \
	$$(wildcard tests/host/$(1)/fake_*.c) $$(wildcard tests/host/$(1)/*.h)
	@mkdir -p $$(dir $$@)
	@$(HOSTCC) $$(HOSTFLAGS) $$(HINC_$(1)) -MM -MP -MT $$@ $(2) tests/host/harness.c \
		tests/host/harness_eq.c $$(HSRC_$(1)) $$(wildcard tests/host/$(1)/fake_*.c) > $$@.d
	$(HOSTCC) $$(HOSTFLAGS) $$(HINC_$(1)) $(2) tests/host/harness.c tests/host/harness_eq.c $$(HSRC_$(1)) \
		$$(wildcard tests/host/$(1)/fake_*.c) -o $$@
HBIN_$(1) += $(HB)/$(1)/$(notdir $(2:.c=))
HDEP += $(HB)/$(1)/$(notdir $(2:.c=)).d
endef

$(foreach l,$(LOTS),$(foreach t,$(HT_$(l)),$(eval $(call host_rule,$(l),$(t)))))
-include $(HDEP)

define host_run
.PHONY: host-$(1)
host-$(1): $$(HBIN_$(1))
	@for t in $$(HBIN_$(1)); do printf 'host %s\n' "$$$$t"; "$$$$t" || exit 1; done
endef

$(foreach l,$(LOTS),$(eval $(call host_run,$(l))))

.PHONY: test-host
test-host: $(foreach l,$(LOTS),host-$(l))
