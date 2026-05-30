# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-05-29
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
    
    # Dynamically parse environment variables from set_env.bat to avoid code duplication
    GET_WIN_VAR = $(subst %~dp0,$(CURDIR)$(SLASH),$(lastword $(subst =, ,$(shell findstr /B /C:"setx $1" set_env.bat))))
    
    export REFALABBIN     := $(call GET_WIN_VAR,REFALABBIN)
    export REFALABLIB     := $(call GET_WIN_VAR,REFALABLIB)
    export REFALABINCLUDE := $(call GET_WIN_VAR,REFALABINCLUDE)
    export REFALABCFLAGS  := $(call GET_WIN_VAR,REFALABCFLAGS)
else
    # Linux / macOS (POSIX) Environment Settings
    SLASH = /
    
    # Dynamically import environment variables from the original set_env bash script
    export REFALABBIN     := $(shell . ./set_env && echo $$REFALABBIN)
    export REFALABLIB     := $(shell . ./set_env && echo $$REFALABLIB)
    export REFALABINCLUDE := $(shell . ./set_env && echo $$REFALABINCLUDE)
    export REFALABCFLAGS  := $(shell . ./set_env && echo $$REFALABCFLAGS)
endif

# Internal source paths for Sub-Makes
DIR_COMP  = src$(SLASH)comp
DIR_INTER = src$(SLASH)inter

# ====================================================================
# 2. MAIN TARGETS
# ====================================================================
.PHONY: all comp inter clean env help

# Default target: Builds the entire project
all: comp inter
	@echo ==================================================
	@echo  Refalab build successfully completed for $(if $(OS),Windows,POSIX)!
	@echo ==================================================

# Build compiler module only (src/comp)
comp:
	@echo >>> Building compiler (comp)...
	$(MAKE) -C $(DIR_COMP)

# Build interpreter module only (src/inter)
inter:
	@echo >>> Building interpreter (inter)...
	$(MAKE) -C $(DIR_INTER)

# ====================================================================
# 3. AUTOMATION & CLEANUP
# ====================================================================

# Centralized clean: invokes inner clean targets from author's subfolders
clean:
	@echo >>> Cleaning temporary files and binaries...
	$(MAKE) -C $(DIR_COMP) clean
	$(MAKE) -C $(DIR_INTER) clean
	@echo >>> Clean completed.

# Display environment variables for path verification
env:
	@echo === Environment variables loaded from set_env ===
	@echo Operating System     : $(if $(OS),Windows,POSIX)
	@echo REFALABBIN           : $(REFALABBIN)
	@echo REFALABLIB           : $(REFALABLIB)
	@echo REFALABINCLUDE       : $(REFALABINCLUDE)
	@echo REFALABCFLAGS        : $(REFALABCFLAGS)

# Built-in help instructions
help:
	@echo Available commands:
	@echo   make       - Build the entire project
	@echo   make comp  - Build the compiler only
	@echo   make inter - Build the interpreter only
	@echo   make clean - Run clean target in all subdirectories
	@echo   make env   - Check how Make loaded paths from set_env
