CC ?= cc
BUILD ?= build-host
CFLAGS ?= -O2 -Wall -Wextra -Werror -Wno-misleading-indentation
INC = -Isrc -I../elx-utils/src -I../elx-ctl/src
COMMON = src/model.c src/demo.c ../elx-ctl/src/commands.c ../elx-utils/src/battery.c
DRAW = src/draw.c ../elx-utils/src/widgets.c ../elx-utils/src/cga.c
all: $(BUILD)/elxdash
$(BUILD)/elxdash: src/main.c src/platform_host.c $(COMMON) $(DRAW) src/dash.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(INC) src/main.c src/platform_host.c $(COMMON) $(DRAW) -o $@
test: all
	$(CC) $(CFLAGS) $(INC) tests/test_model.c src/model.c ../elx-ctl/src/commands.c ../elx-utils/src/battery.c -o $(BUILD)/test-model
	$(BUILD)/test-model
	python3 tests/test_cli.py $(BUILD)/elxdash
