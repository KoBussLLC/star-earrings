PROJECT = star-earrings
BUILD_DIR = bin

CFILES = star_main.c

DEVICE=stm32l011f4ux
OOCD_FILE = target/stm32l0.cfg

VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
