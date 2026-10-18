CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror
BUILD ?= build-host
.PHONY: all test
all: $(BUILD)/elxcap
$(BUILD)/elxcap: src/main.c src/capture.c src/capture.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -Isrc src/main.c src/capture.c -o $@
test: all
	python3 tests/test_capture.py $(BUILD)/elxcap
	$(CC) $(CFLAGS) -Isrc tests/faults.c -o $(BUILD)/test-faults
	$(BUILD)/test-faults
	$(CC) $(CFLAGS) tests/hotkey.c -o $(BUILD)/test-hotkey
	$(BUILD)/test-hotkey
