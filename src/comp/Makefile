# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-06-05
# https://github.com/Aleksandr3Bocharov/refalab

#############################################################################
# Makefile
# Makefile for building of the compiler from RefalAB language
# result - bin/refalabc executable module
# result - bin/refalabc_dbg executable module
#############################################################################

# ====================================================================
# 1. CROSS-PLATFORM & ENVIRONMENT DETECTION
# ====================================================================
CC = clang

ifdef OS
    # Windows Settings
    RM = del /Q /F
    EXE = .exe
    SLASH = \\
    LFLAGS = -pthread -static
    PLATFORM_CFLAGS =
	CLANG_VERSION := $(shell $(CC) -v 2>&1 | powershell -Command "$$input | Select-String 'clang version' | ForEach-Object { ($$_ -split ' ')[2] }")
	CLANG_TARGET := $(shell $(CC) -v 2>&1 | powershell -Command "$$input | Select-String 'Target:' | ForEach-Object { ($$_ -split ' ')[1] }")
    CLANG_THREAD_MODEL := $(shell $(CC) -v 2>&1 | powershell -Command "$$input | Select-String 'Thread model:' | ForEach-Object { ($$_ -split ' ')[2] }")
else
    # Linux / macOS (POSIX) Settings
    RM = rm -f
    EXE =
    SLASH = /
    LFLAGS = 
    PLATFORM_CFLAGS = -DPOSIX
	CLANG_VERSION := $(shell $(CC) -v 2>&1 | sed -n 's/.*clang version \([^ ]*\).*/\1/p')
	CLANG_TARGET := $(shell $(CC) -v 2>&1 | sed -n 's/^Target: //p')
    CLANG_THREAD_MODEL := $(shell $(CC) -v 2>&1 | sed -n 's/^Thread model: //p')
endif

CFLAGS_D = $(PLATFORM_CFLAGS) -DCLANG_VERSION="\"$(CLANG_VERSION)\"" -DCLANG_TARGET_TRIPLE=\"$(CLANG_TARGET)\" -DCLANG_THREAD_MODEL=\"$(CLANG_THREAD_MODEL)\"
CFLAGS_COMMON = -pipe -Wall -O2 $(CFLAGS_D)
CFLAGS_DEBUG  = -pipe -Wall -O0 -g -Dmdebug $(CFLAGS_D)
LINK = clang
REFALABBIN  = ..$(SLASH)..$(SLASH)bin

# ====================================================================
# 2. TARGET DEFINITIONS
# ====================================================================
TARGET_RELEASE = $(REFALABBIN)$(SLASH)refalabc$(EXE)
TARGET_DEBUG   = $(REFALABBIN)$(SLASH)refalabc_dbg$(EXE)

# Source and object files automation
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
OBJS_DBG = $(SRCS:.c=_dbg.o)
HEADERS = $(wildcard *.h)

# ====================================================================
# 3. TARGETS & RULES
# ====================================================================
.PHONY: all clean release debug clean_objs

# Default target: builds both release and debug versions
all: release debug

# --- RELEASE VERSION BUILD ---
release: CFLAGS := $(CFLAGS_COMMON)
release: $(TARGET_RELEASE)

$(TARGET_RELEASE): $(OBJS)
	@echo Linking Release binary: $(TARGET_RELEASE)...
	$(LINK) $(LFLAGS) -o $@ $^

# --- DEBUG VERSION BUILD ---
# Add debugging flags specifically for this target.
debug: CFLAGS := $(CFLAGS_DEBUG)
debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJS_DBG)
	@echo Linking Debug binary: $(TARGET_DEBUG)...
	$(LINK) $(LFLAGS) -o $@ $^

# Template rule to compile any local .c file into .o
%.o: %.c $(HEADERS)
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

# Template rule to compile any local .c file into _dbg.o
%_dbg.o: %.c $(HEADERS)
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

# ====================================================================
# 4. CLEANUP
# ====================================================================
# Helper target to clear object files between release and debug builds
clean_objs:
	$(RM) *.o

# Complete cleanup of all build artifacts and temporary files src/comp
clean: clean_objs
	@echo Cleaning compiler build artifacts...
	$(RM) $(TARGET_RELEASE)
	$(RM) $(TARGET_DEBUG)
	@echo ==================================================
	@echo Full cleanup of src/comp directory completed.
	@echo ==================================================
