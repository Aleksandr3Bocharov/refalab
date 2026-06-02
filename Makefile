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

DIR_BIN = bin
DIR_LIB = lib

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
	@echo '=== Building compiler (comp) ==='
	@$(call MKDIR,$(DIR_BIN))
	$(MAKE) -C $(DIR_COMP)

# Build compiler from RefalAB language only (src/comp)
# --- RELEASE VERSION BUILD ONLY ---
comp-release:
	@echo '=== Building compiler (comp) ==='
	@echo === Building Release binary ===
	@$(call MKDIR,$(DIR_BIN))
	$(MAKE) -C $(DIR_COMP) release

# Build compiler from RefalAB language only (src/comp)
# --- DEBUG VERSION BUILD ONLY ---
comp-debug:
	@echo '=== Building compiler (comp) ==='
	@echo === Building Debug binary ===
	@$(call MKDIR,$(DIR_BIN))
	$(MAKE) -C $(DIR_COMP) debug

# Build interpreter from "jazyk sborki" only (src/inter)
inter:
	@echo '=== Building interpreter (inter) ==='
	@$(call MKDIR,$(DIR_LIB))
	$(MAKE) -C $(DIR_INTER)

# Build interpreter from "jazyk sborki" only (src/inter)
# --- RELEASE VERSION BUILD ONLY ---
inter-release:
	@echo '=== Building interpreter (inter) ==='
	@echo === Building Release components ===
	@$(call MKDIR,$(DIR_LIB))
	$(MAKE) -C $(DIR_INTER) release

# Build interpreter from "jazyk sborki" only (src/inter)
# --- DEBUG VERSION BUILD ONLY ---
inter-debug:
	@echo '=== Building interpreter (inter) ==='
	@echo === Building Debug components ===
	@$(call MKDIR,$(DIR_LIB))
	$(MAKE) -C $(DIR_INTER) debug

# ====================================================================
# 3. AUTOMATION & CLEANUP
# ====================================================================

# Centralized clean: invokes inner clean targets from subfolders
clean:
	@echo === Cleaning temporary files and build artifacts ===
	$(MAKE) -C $(DIR_COMP) clean
	$(MAKE) -C $(DIR_INTER) clean
	@echo === Clean completed. ===

# Complete cleanup of all build artifacts and temporary files src/comp
clean-comp:
	@echo === Cleaning temporary files and build artifacts ===
	$(MAKE) -C $(DIR_COMP) clean
	@echo === Clean completed. ===

# Complete cleanup of all build artifacts and temporary files src/inter
clean-inter:
	@echo === Cleaning temporary files and build artifacts ===
	$(MAKE) -C $(DIR_INTER) clean
	@echo === Clean completed. ===

# Built-in help instructions
help:
	@echo Available commands:
	@echo   make       - Build the entire project
	@echo   make comp  - Build the compiler only
	@echo   make comp-release  - Build the Release binary of the compiler only
	@echo   make comp-debug  - Build the Debug binary of the compiler only 
	@echo   make inter - Build the interpreter only
	@echo   make inter-release  - Build the Release components of the interpreter only
	@echo   make inter-debug  - Build the Debug components of the interpreter only 
	@echo   make clean - Run clean target in all subdirectories
	@echo   make clean-comp - Run clean target in src/comp
	@echo   make clean-inter - Run clean target in src/inter
