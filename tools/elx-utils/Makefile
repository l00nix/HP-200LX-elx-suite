# Build in an isolated directory; never modify an ELKS or Nano-X source tree.
BUILD ?= build-host
HOSTCC ?= cc
WARN = -Wall -Wextra -Werror -Wno-misleading-indentation
CFLAGS = -O2 $(WARN) -Isrc
COMMON = src/status.c src/battery.c

.PHONY: host clean
host: $(BUILD)/elxstat $(BUILD)/elxbat
$(BUILD):
	mkdir -p $@
$(BUILD)/elxstat: src/elxstat.c $(COMMON) src/elx.h | $(BUILD)
	$(HOSTCC) $(CFLAGS) src/elxstat.c $(COMMON) -o $@
$(BUILD)/elxbat: src/elxbat.c src/gauges.c src/widgets.c src/cga.c $(COMMON) src/elx.h src/cga.h src/font.h | $(BUILD)
	$(HOSTCC) $(CFLAGS) src/elxbat.c src/gauges.c src/widgets.c src/cga.c $(COMMON) -o $@
clean:
	rm -f $(BUILD)/elxstat $(BUILD)/elxbat

.PHONY: test
test: host
	python3 tests/test_cli.py $(BUILD)
	$(HOSTCC) $(CFLAGS) -D__ia16__ src/elxbat.c tests/test_power_loop.c -o $(BUILD)/power-loop
	$(BUILD)/power-loop
