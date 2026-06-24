LIB_NAME := libunittest.a
TEST_NAME := test

# Paths
PREFIX := /usr/local
BUILD_DIR := build
SRC_DIR := src
TESTS_DIR := tests

# Toolchain
CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic
AR := ar
ARFLAGS := rcs

# File Discovery
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/src/%.o,$(SRCS))

TESTS_SRCS := $(shell find $(TESTS_DIR) -name '*.c')
TESTS_OBJS := $(patsubst $(TESTS_DIR)/%.c,$(BUILD_DIR)/tests/%.o,$(TESTS_SRCS))

# --- Rules ---

.PHONY: all test clean install uninstall

all: $(BUILD_DIR)/$(LIB_NAME) $(BUILD_DIR)/$(TEST_NAME)

# Target for static library
$(BUILD_DIR)/$(LIB_NAME): $(OBJS)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $(OBJS)

$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

$(BUILD_DIR)/$(TEST_NAME): $(TESTS_OBJS) $(BUILD_DIR)/$(LIB_NAME)
	@mkdir -p $(dir $@)
	$(CC) $(TESTS_OBJS) -L$(BUILD_DIR) -lunittest -o $@

$(BUILD_DIR)/tests/%.o: $(TESTS_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

test: $(BUILD_DIR)/$(TEST_NAME)
	@echo "Running tests..."
	@$<

clean:
	rm -rf $(BUILD_DIR)

install: $(BUILD_DIR)/$(LIB_NAME)
	@echo "Installing $(LIB_NAME) to $(PREFIX)/lib"
	mkdir -p $(PREFIX)/include/unittest
	mkdir -p $(PREFIX)/lib
	cp $(SRC_DIR)/unittest.h $(PREFIX)/include/unittest/
	cp $(BUILD_DIR)/$(LIB_NAME) $(PREFIX)/lib/

uninstall:
	@if [ -z "$(PREFIX)" ] || [ "$(PREFIX)" = "/" ]; then \
		echo "Error: Dangerous PREFIX detected!"; \
		exit 1; \
	fi
	@echo "Uninstalling $(LIB_NAME) from $(PREFIX)/lib"
	rm -f $(PREFIX)/include/unittest/unittest.h
	rm -f $(PREFIX)/lib/$(LIB_NAME)
	rmdir $(PREFIX)/include/unittest 2> /dev/null || true
