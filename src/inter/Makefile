# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-05-30
# https://github.com/Aleksandr3Bocharov/refalab

#############################################################################
# Makefile
# Makefile for interpreter from "jazyk sborki" creation
# result - lib/librefalab.a library
# result - lib/librefalab_dbg.a library
# result - lib/obj/mainrf.o lib/obj/mainrf_dbg.o
# result - lib/obj/rfdbg.o lib/obj/rfdbg_dbg.o
#############################################################################

# ------------------------------------------------------------------------------
# 1. CROSS-PLATFORM ENVIRONMENT AND OS SETTINGS
# ------------------------------------------------------------------------------
ifdef OS
    SLASH = \\
    RM = del /Q /F
    MKDIR = if not exist $(1) mkdir $(1)
    RMDIR = rmdir /S /Q
    PLATFORM_CFLAGS =
    XPLATFORM_SRC = xplatformwin
else
    SLASH = /
    RM = rm -f
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf
    PLATFORM_CFLAGS = -DPOSIX
    XPLATFORM_SRC = xplatformposix
endif

CC = clang
CFLAGS_COMMON = -pipe -Wall -O2 $(PLATFORM_CFLAGS)
CFLAGS_DEBUG  = -pipe -Wall -O2 -g -Dmdebug $(PLATFORM_CFLAGS)
REFALABC = ..$(SLASH)..$(SLASH)bin$(SLASH)refalabc
REFALABCFLAGS = 
AR = ar rcs
REFALAB_LIB = ..$(SLASH)..$(SLASH)lib
OBJ_DIR    = $(REFALAB_LIB)$(SLASH)obj

# ------------------------------------------------------------------------------
# 2. OBJECT FILES LISTS
# ------------------------------------------------------------------------------
# A. Objects for building libraries (interpreter as a library)
LIB_CORE_OBJS = d.o rfintf.o rfrun.o xar.o xbit.o xcf.o xcomp.o xdelf.o xfio.o xio.o xjak.o xmo.o xrand.o xstor.o xsys.o xtime.o xtry.o
LIB_REF_OBJS  = xar2.o xbool.o xcomp2.o xext.o xfio2.o xhof.o xmo2.o xplatform.o xsys2.o
ALL_LIB_OBJS  = $(LIB_CORE_OBJS) $(LIB_REF_OBJS)

REFLIB_RELEASE = $(REFALAB_LIB)$(SLASH)librefalab.a
REFLIB_DEBUG   = $(REFALAB_LIB)$(SLASH)librefalab_dbg.a

# B. Standalone release and debug object files (written directly into lib/obj)
RELEASE_STANDALONE_OBJS = $(OBJ_DIR)$(SLASH)mainrf.o $(OBJ_DIR)$(SLASH)rfdbg.o
DEBUG_STANDALONE_OBJS   = $(OBJ_DIR)$(SLASH)mainrf_dbg.o $(OBJ_DIR)$(SLASH)rfdbg_dbg.o

# ------------------------------------------------------------------------------
# 3. MAIN TARGETS
# ------------------------------------------------------------------------------
.PHONY: all release debug clean clean_objs help

# By default, build both release and debug targets
all: release debug

# Full release build (Library + Standalone release object files in lib/obj)
release: CFLAGS := $(CFLAGS_COMMON)
release: $(ALL_LIB_OBJS) $(RELEASE_STANDALONE_OBJS)
	$(AR) $(REFLIB_RELEASE) $(ALL_LIB_OBJS)
	@echo ==================================================
	@echo Interpreter release components successfully built.
	@echo Library: $(REFLIB_RELEASE)
	@echo Output Folder: $(OBJ_DIR)
	@echo ==================================================

# Full debug build (Debug library + Standalone debug object files in lib/obj)
debug: CFLAGS := $(CFLAGS_DEBUG)
debug: clean_objs $(ALL_LIB_OBJS) $(DEBUG_STANDALONE_OBJS)
	$(AR) $(REFLIB_DEBUG) $(ALL_LIB_OBJS)
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
$(LIB_REF_OBJS): xar2 xbool xcomp2 xext xfio2 xhof xmo2 xplatformposix xplatformwin xsys2
	@echo [Refal-Compiler] Full generation of Refal modules...
	$(REFALABC) xar2 $(REFALABCFLAGS)
	$(REFALABC) xbool $(REFALABCFLAGS)
	$(REFALABC) xcomp2 $(REFALABCFLAGS)
	$(REFALABC) xext $(REFALABCFLAGS)
	$(REFALABC) xfio2 $(REFALABCFLAGS)
	$(REFALABC) xhof $(REFALABCFLAGS)
	$(REFALABC) xmo2 $(REFALABCFLAGS)
	$(REFALABC) $(XPLATFORM_SRC) $(REFALABCFLAGS)
	$(REFALABC) xsys2 $(REFALABCFLAGS)
	@# Copying the platform object depending on the current OS
ifdef OS
	if exist xplatformwin.o copy /Y xplatformwin.o xplatform.o >nul
else
	@if [ -f xplatformposix.o ]; then cp -f xplatformposix.o xplatform.o; fi
endif

# ------------------------------------------------------------------------------
# 5. CLEANUP OPERATIONS
# ------------------------------------------------------------------------------
clean_objs:
	-$(RM) *.o
	@# Remove garbage and temporary listings of the Refal compiler
	-$(RM) *.s
	-$(RM) *.log
	-$(RM) *.lst

clean: clean_objs
	-$(RM) $(REFLIB_RELEASE)
	-$(RM) $(REFLIB_DEBUG)
	-$(call RMDIR,$(OBJ_DIR))
	@echo ==================================================
	@echo Full cleanup of src/inter directory completed.
	@echo ==================================================
