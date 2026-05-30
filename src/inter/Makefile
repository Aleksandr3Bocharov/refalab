# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-05-30
# https://github.com/Aleksandr3Bocharov/refalab

#############################################################################
# Makefile
# Makefile for interpreter library creation
# result - lib/librefalab.a library
# result - lib/librefalab_dbg.a library
# result - mainrf.o mainrf_dbg.o rfdbg.o rfdbg_dbg.o
#############################################################################

# ==============================================================================
# UNIFIED CROSS-PLATFORM MAKEFILE FOR THE INTERPRETER (src/inter)
# Combines inter_lib.*, inter_o.*, delbin, and all refx* automation scripts
# ==============================================================================

# ------------------------------------------------------------------------------
# 1. CROSS-PLATFORM ENVIRONMENT AND OS SETTINGS
# ------------------------------------------------------------------------------
ifeq ($(OS),Windows_NT)
    SLASH = \\
    RM = del /Q /F
    MKDIR = if not exist $(1) mkdir $(1)
    RMDIR = rmdir /S /Q
    PLATFORM_CFLAGS = -D_WIN32
    # Dynamically parse variables from the root set_env.bat
    GET_WIN_VAR = $(shell findstr /R /C:"^set  *$(1)=" ..$(SLASH)..$(SLASH)set_env.bat | for /F "tokens=2 delims==" %%i in ('more') do @echo %%i)
    REFALABC = $(call GET_WIN_VAR,REFALABC)
    REFALABCFLAGS = $(call GET_WIN_VAR,REFALABCFLAGS)
    REFALABCFLAGS_DBG = $(call GET_WIN_VAR,REFALABCFLAGS_DBG)
    XPLATFORM_SRC = xplatformwin.ref
else
    SLASH = /
    RM = rm -f
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf
    PLATFORM_CFLAGS = -DPOSIX
    # Read variables from the root set_env script
    REFALABC := $(shell . ../../set_env && echo $$REFALABC)
    REFALABCFLAGS := $(shell . ../../set_env && echo $$REFALABC)
    REFALABCFLAGS_DBG := $(shell . ../../set_env && echo $$REFALABC_DBG)
    XPLATFORM_SRC = xplatformposix.ref
endif

CC = clang
AR = ar
REFLIB_DIR = ..$(SLASH)..$(SLASH)lib
OBJ_DIR    = $(REFLIB_DIR)$(SLASH)obj

# Compilation flags from original .mak specifications
CFLAGS_COMMON = -pipe -Wall -O2 $(PLATFORM_CFLAGS)
CFLAGS_DEBUG  = -pipe -Wall -O2 -Dmdebug $(PLATFORM_CFLAGS)

# ------------------------------------------------------------------------------
# 2. OBJECT FILES LISTS (STRICTLY ACCORDING TO ALL .MAK FILES)
# ------------------------------------------------------------------------------
# A. Objects for building libraries (interpreter as a library)
LIB_CORE_OBJS = d.o rfintf.o rfrun.o xar.o xbit.o xcf.o xcomp.o xdelf.o xfio.o xio.o xjak.o xmo.o xrand.o xstor.o xsys.o xtime.o xtry.o
LIB_REF_OBJS  = xar2.o xbool.o xcomp2.o xext.o xfio2.o xhof.o xmo2.o xplatform.o xsys2.o
ALL_LIB_OBJS  = $(LIB_CORE_OBJS) $(LIB_REF_OBJS)

# B. Standalone release and debug object files (written directly into lib/obj)
RELEASE_STANDALONE_OBJS = $(OBJ_DIR)$(SLASH)mainrf.o $(OBJ_DIR)$(SLASH)rfdbg.o
DEBUG_STANDALONE_OBJS   = $(OBJ_DIR)$(SLASH)mainrf_dbg.o $(OBJ_DIR)$(SLASH)rfdbg_dbg.o

REFLIB_RELEASE = $(REFLIB_DIR)$(SLASH)librefalab.a
REFLIB_DEBUG   = $(REFLIB_DIR)$(SLASH)librefalab_dbg.a

# ------------------------------------------------------------------------------
# 3. MAIN TARGETS
# ------------------------------------------------------------------------------
.PHONY: all release debug clean clean_objs help

# By default, build both release and debug targets
all: release debug

# Full release build (Library + Standalone release object files in lib/obj)
release: CFLAGS := $(CFLAGS_COMMON)
release: $(ALL_LIB_OBJS) $(RELEASE_STANDALONE_OBJS)
	$(AR) rs $(REFLIB_RELEASE) $(ALL_LIB_OBJS)
	@echo ==================================================
	@echo Interpreter release components successfully built.
	@echo Library: $(REFLIB_RELEASE)
	@echo Output Folder: $(OBJ_DIR)
	@echo ==================================================

# Full debug build (Debug library + Standalone debug object files in lib/obj)
debug: CFLAGS := $(CFLAGS_DEBUG)
debug: REFALABCFLAGS := $(REFALABCFLAGS_DBG)
debug: clean_objs $(ALL_LIB_OBJS) $(DEBUG_STANDALONE_OBJS)
	$(AR) rs $(REFLIB_DEBUG) $(ALL_LIB_OBJS)
	@echo ==================================================
	@echo Interpreter debug components successfully built.
	@echo Debug Library: $(REFLIB_DEBUG)
	@echo Output Folder: $(OBJ_DIR)
	@echo ==================================================

# ------------------------------------------------------------------------------
# 4. COMPILATION AND AUTOMATIC GENERATION RULES
# ------------------------------------------------------------------------------

# Standard rule for library C files compilation (creates objects locally)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Explicit rules for standalone release objects (compiled directly into lib/obj)
$(OBJ_DIR)$(SLASH)mainrf.o: mainrf.c
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)$(SLASH)rfdbg.o: rfdbg.c
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

# Explicit rules for standalone debug objects (compiled directly into lib/obj)
$(OBJ_DIR)$(SLASH)mainrf_dbg.o: mainrf.c
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)$(SLASH)rfdbg_dbg.o: rfdbg.c
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

# Group rule for modules generation from .ref files via Refal compiler
$(LIB_REF_OBJS): xar2.ref xbool.ref xcomp2.ref xext.ref xfio2.ref xhof.ref xmo2.ref xplatformposix.ref xplatformwin.ref xsys2.ref
	@echo [Refal-Compiler] Full generation of Refal modules...
	$(REFALABC) $(REFALABCFLAGS) xar2.ref
	$(REFALABC) $(REFALABCFLAGS) xbool.ref
	$(REFALABC) $(REFALABCFLAGS) xcomp2.ref
	$(REFALABC) $(REFALABCFLAGS) xext.ref
	$(REFALABC) $(REFALABCFLAGS) xfio2.ref
	$(REFALABC) $(REFALABCFLAGS) xhof.ref
	$(REFALABC) $(REFALABCFLAGS) xmo2.ref
	$(REFALABC) $(REFALABCFLAGS) $(XPLATFORM_SRC)
	$(REFALABC) $(REFALABCFLAGS) xsys2.ref
	@# Copying the platform object depending on the current OS
ifeq ($(OS),Windows_NT)
	if exist xplatformwin.o copy /Y xplatformwin.o xplatform.o >nul
else
	@if [ -f xplatformposix.o ]; then cp -f xplatformposix.o xplatform.o; fi
endif

# ------------------------------------------------------------------------------
# 5. CLEANUP OPERATIONS (FULL REPLACEMENT FOR DELBIN SCRIPTS)
# ------------------------------------------------------------------------------
clean_objs:
	-$(RM) *.o
	@# Remove garbage and temporary listings of the Refal compiler
	-$(RM) *.s
	-$(RM) *.log
	-$(RM) *.lst
	@# Remove intermediate C files if they were generated
	-$(RM) xar2.c xbool.c xcomp2.c xext.c xfio2.c xhof.c xmo2.c xplatformposix.c xplatformwin.c xsys2.c

clean: clean_objs
	-$(RM) $(REFLIB_RELEASE)
	-$(RM) $(REFLIB_DEBUG)
	-$(call RMDIR,$(OBJ_DIR))
	@echo ==================================================
	@echo Full cleanup of src/inter directory completed.
	@echo ==================================================
