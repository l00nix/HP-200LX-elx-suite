CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror
BUILD ?= build-host
.PHONY: all test
all: $(BUILD)/elxctl
$(BUILD)/elxctl: src/main.c src/commands.c src/hardware.c src/status.c src/ctl.h src/elx.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Isrc src/main.c src/commands.c src/hardware.c src/status.c -o $@
test: all
	python3 tests/test_cli.py $(BUILD)/elxctl
	$(CC) $(CFLAGS) -Isrc tests/commands.c src/commands.c -o $(BUILD)/test-commands
	$(BUILD)/test-commands
