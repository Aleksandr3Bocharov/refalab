# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-06-09
# https://github.com/Aleksandr3Bocharov/refalab

#############################################################################
# Makefile
# Makefile for interpreter from "jazyk sborki" creation
# result - lib/librefalab.a library
# result - lib/librefalab_dbg.a library
# result - lib/obj/refalab_initiator.o lib/obj/refalab_initiator_dbg.o
# result - lib/obj/refalab_debugger.o lib/obj/refalab_debugger_dbg.o
#############################################################################

# ------------------------------------------------------------------------------
# 1. CROSS-PLATFORM ENVIRONMENT AND OS SETTINGS
# ------------------------------------------------------------------------------
ifdef OS
    SLASH = \\
    RM = del /Q /F
    MKDIR = if not exist $(1) mkdir $(1)
    RMDIR = rmdir /S /Q $(1)
    PLATFORM_CFLAGS =
    XPLATFORM_SRC = xplatform_win
else
    SLASH = /
    RM = rm -f
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf $(1)
    PLATFORM_CFLAGS = -DPOSIX
    XPLATFORM_SRC = xplatform_posix
endif

CC = clang
CFLAGS_COMMON = -pipe -Wall -O2 $(PLATFORM_CFLAGS)
CFLAGS_DEBUG  = -pipe -Wall -O0 -g -Dmdebug $(PLATFORM_CFLAGS)
REFALABC = ..$(SLASH)..$(SLASH)bin$(SLASH)refalabc
AR = ar rcs
REFALAB_LIB = ..$(SLASH)..$(SLASH)lib
OBJ_DIR    = $(REFALAB_LIB)$(SLASH)obj

# ------------------------------------------------------------------------------
# 2. OBJECT FILES LISTS
# ------------------------------------------------------------------------------
# A. Objects for building libraries (interpreter as a library)
STANDALONE_SRCS = initiator.c debugger.c
LIB_CORE_SRCS = $(filter-out $(STANDALONE_SRCS), $(wildcard *.c))
LIB_CORE_OBJS = $(LIB_CORE_SRCS:.c=.o)
LIB_CORE_OBJS_DBG = $(LIB_CORE_SRCS:.c=_dbg.o)
XPLATFORM_SRCS = xplatform_win.ref xplatform_posix.ref
REF_MODULES  := $(filter-out $(XPLATFORM_SRCS), $(wildcard *.ref))
LIB_REF_OBJS := $(REF_MODULES:.ref=.o) xplatform.o
LIB_REF_OBJS_DBG := $(REF_MODULES:.ref=_dbg.o) xplatform_dbg.o
ALL_LIB_OBJS  = $(LIB_CORE_OBJS) $(LIB_REF_OBJS)
ALL_LIB_OBJS_DBG  = $(LIB_CORE_OBJS_DBG) $(LIB_REF_OBJS_DBG)
HEADERS = $(wildcard *.h)

REFLIB_RELEASE = $(REFALAB_LIB)$(SLASH)librefalab.a
REFLIB_DEBUG   = $(REFALAB_LIB)$(SLASH)librefalab_dbg.a

# B. Standalone release and debug object files (written directly into lib/obj)
RELEASE_STANDALONE_OBJS = $(OBJ_DIR)$(SLASH)refalab_initiator.o $(OBJ_DIR)$(SLASH)refalab_debugger.o
DEBUG_STANDALONE_OBJS   = $(OBJ_DIR)$(SLASH)refalab_initiator_dbg.o $(OBJ_DIR)$(SLASH)refalab_debugger_dbg.o

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
debug: $(ALL_LIB_OBJS_DBG) $(DEBUG_STANDALONE_OBJS)
	$(AR) $(REFLIB_DEBUG) $(ALL_LIB_OBJS_DBG)
	@echo ==================================================
	@echo Interpreter debug components successfully built.
	@echo Debug Library: $(REFLIB_DEBUG)
	@echo Output Folder: $(OBJ_DIR)
	@echo ==================================================

# ------------------------------------------------------------------------------
# 4. COMPILATION AND AUTOMATIC GENERATION RULES
# ------------------------------------------------------------------------------

# Standard rule for library C files compilation (creates objects locally)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Standard rule for library C files compilation (creates _dbg objects locally)
%_dbg.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Explicit rules for standalone release objects (compiled directly into lib/obj)
$(OBJ_DIR)$(SLASH)refalab_initiator.o: initiator.c $(HEADERS)
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)$(SLASH)refalab_debugger.o: debugger.c $(HEADERS)
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

# Explicit rules for standalone debug objects (compiled directly into lib/obj)
$(OBJ_DIR)$(SLASH)refalab_initiator_dbg.o: initiator.c $(HEADERS)
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)$(SLASH)refalab_debugger_dbg.o: debugger.c $(HEADERS)
	@$(call MKDIR,$(OBJ_DIR))
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for all standard Refal modules
%.o: %.ref
	$(REFALABC) $* -l,"-O2 -Wno-override-module" > $*.log

# Special rule for platform-dependent xplatform.o
xplatform.o: $(XPLATFORM_SRC).ref
	$(REFALABC) $(XPLATFORM_SRC) -l,"-o $@ -O2 -Wno-override-module" > xplatform.log

# Pattern rule for all standard Refal modules
%_dbg.o: %.ref
	$(REFALABC) $* -l,"-o $@ -O0 -g -Wno-override-module" > $*_dbg.log

# Special rule for platform-dependent xplatform.o
xplatform_dbg.o: $(XPLATFORM_SRC).ref
	$(REFALABC) $(XPLATFORM_SRC) -l,"-o $@ -O0 -g -Wno-override-module" > xplatform_dbg.log

# ------------------------------------------------------------------------------
# 5. CLEANUP OPERATIONS
# ------------------------------------------------------------------------------
clean_objs:
	-$(RM) *.o
	@echo Remove garbage and temporary listings of the RefalAB compiler
	-$(RM) *.ll
	-$(RM) *.log
	-$(RM) *.lst

clean: clean_objs
	@echo Cleaning interpreter build artifacts...
	-$(RM) $(REFLIB_RELEASE)
	-$(RM) $(REFLIB_DEBUG)
	-$(call RMDIR,$(OBJ_DIR))
	@echo ==================================================
	@echo Full cleanup of src/inter directory completed.
	@echo ==================================================
