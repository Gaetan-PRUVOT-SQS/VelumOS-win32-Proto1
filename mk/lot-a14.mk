LOTS += a14
DIRS_a14 := user/include user/libvelum user/apps/vtest tests/host/a14
HT_a14 := $(sort $(wildcard tests/host/a14/test_*.c))
A14_HOST_EXCLUDE := user/libvelum/crt_main.c user/libvelum/crt_fail.c \
	user/libvelum/printf.c
HSRC_a14 := $(filter-out $(A14_HOST_EXCLUDE),$(sort $(wildcard user/libvelum/*.c))) \
	user/crt/bsearch.S tests/host/a14/real_stub.S tests/host/a14/real_tls.S \
	lib/libk/fmt.c lib/libk/fmt_num.c lib/libk/fmt_out.c \
	lib/libk/fmt_parse.c lib/libk/fmt_str.c lib/libk/str.c lib/libk/str2.c \
	lib/libk/memcmp.c
A14_HOST_RENAME := malloc free calloc realloc reallocarray abs labs llabs atoi \
	atol strtol strtoll strtoul strtoull qsort bsearch rand srand atexit exit \
	abort snprintf vsnprintf isalnum isalpha isblank iscntrl isdigit isgraph \
	islower isprint ispunct isspace isupper isxdigit tolower toupper strdup \
	strndup explicit_bzero
HINC_a14 := -Iuser/libvelum -iquote user/include \
	$(foreach f,$(A14_HOST_RENAME),-D$(f)=vu_$(f))
ULIBS += velum
ULIB_velum_LOT := a14
ULIB_velum_SRC := $(sort $(wildcard user/libvelum/*.c)) \
	$(sort $(wildcard user/crt/*.S)) \
	$(sort $(wildcard lib/libk/*.c)) lib/libk/mem.S
UAPPS += vtest
UAPP_vtest_LOT := a14
UAPP_vtest_SRC := $(sort $(wildcard user/apps/vtest/*.c))
UAPP_vtest_LIBS := velum

.PHONY: a14-hdrcheck
a14-hdrcheck:
	@for h in $$(cd user/include && find . -name '*.h' | sort); do \
		printf '#include "%s"\n' $${h#./} | \
		$(UCC) $(UCFLAGS) -x c -fsyntax-only - || exit 1; \
	done; echo 'a14 : en-têtes user autonomes : OK'

.PHONY: a14-elf
a14-elf: $(O)/root/system/bin/vtest
	python3 tests/host/a14/check_elf.py $(O)/root/system/bin/vtest
