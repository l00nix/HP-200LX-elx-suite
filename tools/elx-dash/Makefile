CC ?= cc
BUILD ?= build-host
CFLAGS ?= -O2 -Wall -Wextra -Werror -Wno-misleading-indentation
INC = -Isrc -I../elx-utils/src -I../elx-ctl/src
COMMON = src/model.c src/menu.c src/charge.c src/charge_host.c src/demo.c ../elx-ctl/src/commands.c ../elx-utils/src/battery.c
DRAW = src/draw.c ../elx-utils/src/widgets.c ../elx-utils/src/cga.c
all: $(BUILD)/elxdash
$(BUILD)/elxdash: src/main.c src/platform_host.c $(COMMON) $(DRAW) src/dash.h
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(INC) src/main.c src/platform_host.c $(COMMON) $(DRAW) -o $@
test: all
	$(CC) $(CFLAGS) $(INC) tests/test_model.c src/model.c src/menu.c src/charge.c src/charge_host.c $(DRAW) ../elx-ctl/src/commands.c ../elx-utils/src/battery.c -o $(BUILD)/test-model
	$(BUILD)/test-model
	$(CC) $(CFLAGS) -Isrc tests/test_charge.c src/charge.c -o $(BUILD)/test-charge
	$(BUILD)/test-charge
	python3 tests/test_service.py
	python3 tests/test_cli.py $(BUILD)/elxdash
	$(CC) $(CFLAGS) $(INC) tests/power_preview.c src/draw.c src/menu.c src/charge.c src/charge_host.c src/model.c ../elx-ctl/src/commands.c ../elx-utils/src/gauges.c ../elx-utils/src/widgets.c ../elx-utils/src/cga.c ../elx-utils/src/status.c ../elx-utils/src/battery.c -o $(BUILD)/power-preview
	python3 tests/test_power.py $(BUILD)/power-preview

	$(CC) $(CFLAGS) $(INC) tests/update_preview.c src/draw.c src/menu.c src/charge.c src/charge_host.c src/model.c ../elx-ctl/src/commands.c ../elx-utils/src/widgets.c ../elx-utils/src/cga.c ../elx-utils/src/battery.c -o $(BUILD)/update-preview
	python3 tests/test_update.py $(BUILD)/update-preview
