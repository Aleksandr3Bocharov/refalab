# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-05-29
# https://github.com/Aleksandr3Bocharov/refalab

# ====================================================================
# 1. CROSS-PLATFORM & ENVIRONMENT DETECTION
# ====================================================================
ifdef OS
    # Windows Settings (Based on comp.Windows.mak and delbin.bat)
    RM = del /Q /F
    EXE = .exe
    SLASH = \\
    CC = clang
    LINK = clang
    LFLAGS = -pthread -static
    PLATFORM_FLAGS =
else
    # Linux / macOS (POSIX) Settings (Based on comp.POSIX.mak and delbin)
    RM = rm -f
    EXE =
    SLASH = /
    CC = clang
    LINK = clang
    LFLAGS = 
    PLATFORM_FLAGS = -DPOSIX
endif

# Fallback absolute paths relative to src/comp directory if not inherited from root
REFALABBIN     ?= ..$(SLASH)..$(SLASH)bin
REFALABINCLUDE ?= ..$(SLASH)..$(SLASH)include
REFALABCFLAGS  ?= -pipe -Wall -O2

# Base compiler flags shared between both versions
BASE_CFLAGS = $(REFALABCFLAGS) $(PLATFORM_FLAGS)

# ====================================================================
# 2. TARGET DEFINITIONS
# ====================================================================
TARGET_RELEASE = $(REFALABBIN)$(SLASH)refalabc$(EXE)
TARGET_DEBUG   = $(REFALABBIN)$(SLASH)refalabc_dbg$(EXE)

# Source and object files automation
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

# ====================================================================
# 3. TARGETS & RULES
# ====================================================================
.PHONY: all clean release debug clean_objs

# Default target: builds both release and debug versions
all: release debug

# --- RELEASE VERSION BUILD ---
# Override CFLAGS specifically for this target and its prerequisites
release: CFLAGS := $(BASE_CFLAGS)
release: $(TARGET_RELEASE)

$(TARGET_RELEASE): $(OBJS)
	@echo Linking Release binary: $(TARGET_RELEASE)...
	$(LINK) $(LFLAGS) -o $@ $^

# --- DEBUG VERSION BUILD ---
# Add debugging flags specifically for this target.
# Forces object file cleanup before building to ensure re-compilation with debug flags.
debug: CFLAGS := $(BASE_CFLAGS) -g -Dmdebug
debug: clean_objs $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJS)
	@echo Linking Debug binary: $(TARGET_DEBUG)...
	$(LINK) $(LFLAGS) -o $@ $^

# Template rule to compile any local .c file into .o
%.o: %.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

# ====================================================================
# 4. CLEANUP (Integration of delbin / delbin.bat)
# ====================================================================
# Helper target to clear object files between release and debug builds
clean_objs:
	$(RM) *.o

# Complete cleanup of all build artifacts and temporary files
clean: clean_objs
	@echo Cleaning compiler build artifacts...
	$(RM) $(TARGET_RELEASE)
	$(RM) $(TARGET_DEBUG)

