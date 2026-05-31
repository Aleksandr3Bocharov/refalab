# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-05-30
# https://github.com/Aleksandr3Bocharov/refalab

#############################################################################
# Makefile
# Makefile for building RefalAB system
#############################################################################

# ====================================================================
# 1. CROSS-PLATFORM BLOCK (OS & Shell Commands Detection)
# ====================================================================
ifdef OS
    # Windows Environment Settings
    SLASH = \\
	MKDIR = if not exist $(1) mkdir $(1)
else
    # Linux / macOS (POSIX) Environment Settings
    SLASH = /
	MKDIR = mkdir -p $(1)
endif

# Internal source paths for Sub-Makes
DIR_COMP  = src$(SLASH)comp
DIR_INTER = src$(SLASH)inter

BIN_DIR = bin
LIB_DIR = lib

# ====================================================================
# 2. MAIN TARGETS
# ====================================================================
.PHONY: all comp inter clean env help

# Default target: Builds the entire project
all: comp inter
	@echo =======================================================
	@echo  RefalAB build successfully completed for $(if $(OS),Windows,POSIX)!
	@echo =======================================================

# Build compiler from RefalAB language only (src/comp)
comp:
	@echo === Building compiler (comp) ===
	@$(call MKDIR,$(BIN_DIR))
	$(MAKE) -C $(DIR_COMP)

# Build interpreter from "jazyk sborki" (src/inter)
inter:
	@echo === Building interpreter (inter) ===
	@$(call MKDIR,$(LIB_DIR))
	$(MAKE) -C $(DIR_INTER)

# ====================================================================
# 3. AUTOMATION & CLEANUP
# ====================================================================

# Centralized clean: invokes inner clean targets from subfolders
clean:
	@echo === Cleaning temporary files and binaries ===
	$(MAKE) -C $(DIR_COMP) clean
	$(MAKE) -C $(DIR_INTER) clean
	@echo === Clean completed. ===

# Built-in help instructions
help:
	@echo Available commands:
	@echo   make       - Build the entire project
	@echo   make comp  - Build the compiler only
	@echo   make inter - Build the interpreter only
	@echo   make clean - Run clean target in all subdirectories
