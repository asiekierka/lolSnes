# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Antonio Niño Díaz, 2023

# User config
# ===========

NAME		:= lolSnes

GAME_TITLE	:= lolSnes
GAME_SUBTITLE	:= A SNES emulator for
GAME_AUTHOR	:= your Nintendo DS
GAME_ICON	:= lolsnes.bmp

# DLDI and internal SD slot of DSi
# --------------------------------

# Root folder of the SD image
SDROOT		:= sdroot
# Name of the generated image it "DSi-1.sd" for no$gba in DSi mode
SDIMAGE		:= image.bin

# Source code paths
# -----------------

NITROFATDIR	:=

# Tools
# -----

MAKE		:= make
RM		:= rm -rf

# Verbose flag
# ------------

ifeq ($(VERBOSE),1)
V		:=
else
V		:= @
endif

# Directories
# -----------

ARM9DIR		:= arm9
ARM7DIR		:= arm7

# Build artfacts
# --------------

NITROFAT_IMG	:= build/nitrofat.bin
ROM		:= $(NAME).nds

# Targets
# -------

.PHONY: all clean arm9 arm7 dldipatch sdimage

all: $(ROM)

clean:
	@echo "  CLEAN"
	$(V)$(MAKE) -f Makefile.arm9 clean --no-print-directory
	$(V)$(MAKE) -f Makefile.arm7 clean --no-print-directory
	$(V)$(RM) $(ROM) build $(SDIMAGE)

arm9:
	$(V)+$(MAKE) -f Makefile.arm9 --no-print-directory

arm7:
	$(V)+$(MAKE) -f Makefile.arm7 --no-print-directory

ifneq ($(strip $(NITROFATDIR)),)
# Additional arguments for ndstool
NDSTOOL_FAT	:= -F $(NITROFAT_IMG)

$(NITROFAT_IMG): $(NITROFATDIR)
	@echo "  IMGBUILD $@ $(NITROFATDIR)"
	$(V)sh $(BLOCKSDS)/tools/imgbuild/imgbuild.sh $@ $(NITROFATDIR)

# Combine the title strings
ifeq ($(strip $(GAME_SUBTITLE)),)
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_AUTHOR)
else
    GAME_FULL_TITLE := $(GAME_TITLE);$(GAME_SUBTITLE);$(GAME_AUTHOR)
endif

# Make the NDS ROM depend on the filesystem image only if it is needed
$(ROM): $(NITROFAT_IMG)
endif

$(ROM): arm9 arm7
	@echo "  NDSTOOL $@"
	$(V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-7 build/arm7.elf -9 build/arm9.elf \
		-b $(GAME_ICON) "$(GAME_FULL_TITLE)" \
		$(NDSTOOL_FAT)

sdimage:
	@echo "  IMGBUILD $(SDIMAGE) $(SDROOT)"
	$(V)sh $(BLOCKSDS)/tools/imgbuild/imgbuild.sh $(SDIMAGE) $(SDROOT)

dldipatch: $(ROM)
	@echo "  DLDITOOL $(ROM)"
	$(V)$(BLOCKSDS)/tools/dlditool/dlditool \
		$(BLOCKSDS)/tools/dldi/r4tfv2.dldi $(ROM)
