LOTS += a05
A05_KDIRS := arch/x86_64/acpi arch/x86_64/apic arch/x86_64/tsc \
	arch/x86_64/hpet arch/x86_64/pit arch/x86_64/rtc arch/x86_64/power \
	kernel/irq kernel/time
KSRC_a05 := $(sort $(foreach d,$(A05_KDIRS),$(wildcard $(d)/*.c $(d)/*.S)))
DIRS_a05 := $(A05_KDIRS) tests/host/a05
HT_a05 := $(sort $(wildcard tests/host/a05/test_*.c))
HSRC_a05 := arch/x86_64/acpi/acpi_util.c arch/x86_64/acpi/acpi_rsdp.c \
	arch/x86_64/acpi/acpi_sdt.c arch/x86_64/acpi/acpi_madt.c \
	arch/x86_64/acpi/acpi_madt2.c arch/x86_64/acpi/acpi_fadt.c \
	arch/x86_64/acpi/acpi_fadt2.c arch/x86_64/acpi/acpi_s5.c \
	arch/x86_64/acpi/acpi_tables.c arch/x86_64/apic/ioapic_enc.c \
	arch/x86_64/rtc/rtc_codec.c arch/x86_64/rtc/rtc_hour.c \
	kernel/irq/irq_core.c kernel/irq/irq_core2.c kernel/irq/irq_route.c \
	kernel/time/tmath.c kernel/time/civil.c kernel/time/civil2.c \
	kernel/time/theap.c kernel/time/theap_sift.c kernel/time/wait.c \
	kernel/time/sys_check.c lib/libk/str.c lib/libk/str2.c lib/libk/memcmp.c
HINC_a05 := -Iarch/x86_64/acpi -Iarch/x86_64/apic -Iarch/x86_64/rtc \
	-Ikernel/irq -Ikernel/time
