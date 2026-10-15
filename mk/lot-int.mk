LOTS += int
KSRC_int :=
DIRS_int := user/apps/spin
UAPPS += spin
UAPP_spin_LOT := int
UAPP_spin_SRC := $(sort $(wildcard user/apps/spin/*.c))
UAPP_spin_LIBS := velum
