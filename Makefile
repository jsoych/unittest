LIB_NAME := libunittest.a

# Paths
PREFIX := /usr/local
BUILD_DIR := build
SRC_DIR := src

# Toolchain
CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic
AR := ar
ARFLAGS := rcs

# File Discovery
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# --- Rules ---

$(LIB_NAME): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build/* libunittest.a

install: $(LIB_NAME)
	@echo "Installing $(LIB_NAME) to $(PREFIX)..."
	mkdir -p $(PREFIX)/include/unittest
	mkdir -p $(PREFIX)/lib
	cp $(SRC_DIR)/unittest.h $(PREFIX)/include/unittest/
	cp $(LIB_NAME) $(PREFIX)/lib/

.PHONY: clean install
