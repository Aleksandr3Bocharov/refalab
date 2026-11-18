# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-11-17
# https://github.com/Aleksandr3Bocharov/refalab

#############################################################################
# Makefile
# Top-level makefile of the RefalAB system.
# It only dispatches to the sub-makefiles:
#   src/comp  - the compiler (written in RefalAB), result: bin/
#   src/inter - the interpreter and the standard library modules, result: lib/
# Run 'make help' for the list of targets.
#############################################################################

# ====================================================================
# 1. CROSS-PLATFORM BLOCK (OS & Shell Commands Detection)
# ====================================================================
ifdef OS
    # Windows: path separator and a mkdir that tolerates an existing directory
    SLASH = \\
	MKDIR = if not exist $(1) mkdir $(1)
else
    # Linux / macOS (POSIX): path separator and mkdir -p
    SLASH = /
	MKDIR = mkdir -p $(1)
endif

# Source directories of the sub-makes
DIR_COMP  = src$(SLASH)comp
DIR_INTER = src$(SLASH)inter

# Output directories: executables (bin) and library objects (lib)
DIR_BIN = bin
DIR_LIB = lib

# ====================================================================
# 2. MAIN TARGETS
# ====================================================================
.PHONY: all comp inter clean env help

# Default target: builds the whole project (compiler and interpreter)
all: comp inter
	@echo =======================================================
	@echo  RefalAB build successfully completed for $(if $(OS),Windows,POSIX)!
	@echo =======================================================

# Build the compiler only (src/comp): release and debug binaries
comp:
	@echo '=== Building compiler (comp) ==='
	@$(call MKDIR,$(DIR_BIN))
	$(MAKE) -C $(DIR_COMP)

# Build only the release binary of the compiler (src/comp)
comp-release:
	@echo '=== Building compiler (comp) ==='
	@echo === Building Release binary ===
	@$(call MKDIR,$(DIR_BIN))
	$(MAKE) -C $(DIR_COMP) release

# Build only the debug binary of the compiler (src/comp, -Dmdebug trace output)
comp-debug:
	@echo '=== Building compiler (comp) ==='
	@echo === Building Debug binary ===
	@$(call MKDIR,$(DIR_BIN))
	$(MAKE) -C $(DIR_COMP) debug

# Build the interpreter only (src/inter): release and debug components
inter:
	@echo '=== Building interpreter (inter) ==='
	@$(call MKDIR,$(DIR_LIB))
	$(MAKE) -C $(DIR_INTER)

# Build only the release components of the interpreter (src/inter)
inter-release:
	@echo '=== Building interpreter (inter) ==='
	@echo === Building Release components ===
	@$(call MKDIR,$(DIR_LIB))
	$(MAKE) -C $(DIR_INTER) release

# Build only the debug components of the interpreter (src/inter, -Dmdebug trace output)
inter-debug:
	@echo '=== Building interpreter (inter) ==='
	@echo === Building Debug components ===
	@$(call MKDIR,$(DIR_LIB))
	$(MAKE) -C $(DIR_INTER) debug

# ====================================================================
# 3. AUTOMATION & CLEANUP
# ====================================================================

# Clean everything: runs the clean target of both sub-makes
clean:
	@echo === Cleaning temporary files and build artifacts ===
	$(MAKE) -C $(DIR_COMP) clean
	$(MAKE) -C $(DIR_INTER) clean
	@echo === Clean completed. ===

# Remove build artifacts and temporary files of the compiler (src/comp)
clean-comp:
	@echo === Cleaning temporary files and build artifacts ===
	$(MAKE) -C $(DIR_COMP) clean
	@echo === Clean completed. ===

# Remove build artifacts and temporary files of the interpreter (src/inter)
clean-inter:
	@echo === Cleaning temporary files and build artifacts ===
	$(MAKE) -C $(DIR_INTER) clean
	@echo === Clean completed. ===

# Print the list of available targets
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
