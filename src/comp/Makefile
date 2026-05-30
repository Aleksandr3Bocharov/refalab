# Copyright (c) 2026 Aleksandr Bocharov
# SPDX-License-Identifier: MIT
# 2026-05-30
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
ifdef OS
    # Windows Settings
    RM = del /Q /F
    EXE = .exe
    SLASH = \\
    LFLAGS = -pthread -static
    PLATFORM_CFLAGS =
else
    # Linux / macOS (POSIX) Settings
    RM = rm -f
    EXE =
    SLASH = /
    LFLAGS = 
    PLATFORM_CFLAGS = -DPOSIX
endif

CC = clang
CFLAGS = -pipe -Wall -O2 $(PLATFORM_CFLAGS)
DEBUG_FLAGS = -g -Dmdebug
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

# ====================================================================
# 3. TARGETS & RULES
# ====================================================================
.PHONY: all clean release debug clean_objs

# Default target: builds both release and debug versions
all: release debug

# --- RELEASE VERSION BUILD ---
release: $(TARGET_RELEASE)

$(TARGET_RELEASE): $(OBJS)
	@echo Linking Release binary: $(TARGET_RELEASE)...
	$(LINK) $(LFLAGS) -o $@ $^

# --- DEBUG VERSION BUILD ---
# Add debugging flags specifically for this target.
# Forces object file cleanup before building to ensure re-compilation with debug flags.
debug: CFLAGS := $(CFLAGS) $(DEBUG_CFLAGS)
debug: clean_objs $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJS)
	@echo Linking Debug binary: $(TARGET_DEBUG)...
	$(LINK) $(LFLAGS) -o $@ $^

# Template rule to compile any local .c file into .o
%.o: %.c
	@echo Compiling $<...
	$(CC) $(CFLAGS) -c $< -o $@

# ====================================================================
# 4. CLEANUP
# ====================================================================
# Helper target to clear object files between release and debug builds
clean_objs:
	$(RM) *.o

# Complete cleanup of all build artifacts and temporary files
clean: clean_objs
	@echo Cleaning compiler build artifacts...
	$(RM) $(TARGET_RELEASE)
	$(RM) $(TARGET_DEBUG)

