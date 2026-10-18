BUILD ?= build-host
CC ?= cc
CFLAGS ?= -O2 -Wall -Wextra -Werror -Wno-misleading-indentation -Isrc
SOURCES=src/main.c src/files.c src/terminal.c
.PHONY: host
host: $(BUILD)/elxcmd
$(BUILD)/elxcmd: $(SOURCES) src/elxcmd.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(SOURCES) -o $@
