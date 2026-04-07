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
	rm -rf $(BUILD_DIR) libunittest.a

install: $(LIB_NAME)
	@echo "Installing $(LIB_NAME) to $(PREFIX)/lib"
	mkdir -p $(PREFIX)/include/unittest
	mkdir -p $(PREFIX)/lib
	cp $(SRC_DIR)/unittest.h $(PREFIX)/include/unittest/
	cp $(LIB_NAME) $(PREFIX)/lib/

uninstall:
	@if [ -z "$(PREFIX)" ] || [ "$(PREFIX)" = "/" ]; then \
		echo "Error: Dangerous PREFIX detected!"; \
		exit 1; \
	fi
	@echo "Uninstalling $(LIB_NAME) from $(PREFIX)/lib"
	rm -f $(PREFIX)/include/unittest/unittest.h
	rm -f $(PREFIX)/lib/$(LIB_NAME)
	rmdir $(PREFIX)/include/unittest 2> /dev/null || true

.PHONY: clean install uninstall
