# Variables
CC = gcc
NAME = jrrom-parkour
CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Wextra -Wmissing-prototypes -Wstrict-prototypes

# Directories
BUILD_DIR := build
RAYLIB_DIR := raylib/src

.PHONY: raylib clean

all: build

# Automatically static
raylib:
	$(MAKE) -C $(RAYLIB_DIR) PLATFORM=PLATFORM_DESKTOP

build: raylib
	$(CC) src/*.c \
		$(CFLAGS) \
		-I$(RAYLIB_DIR) -L$(RAYLIB_DIR) \
		-lraylib -lGL -lm -lpthread -ldl -lrt -lX11 \
		-o $(BUILD_DIR)/$(NAME)

clean:
	$(MAKE) -C $(RAYLIB_DIR) clean
	rm -rf $(BUILD_DIR)/*

