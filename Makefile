.PHONY: all elks test install
all: elks
elks:
	sh build.sh
test:
	$(MAKE) -C tools/elx-utils test
	$(MAKE) -C tools/elx-ctl test
	$(MAKE) -C tools/elx-dash test
	python3 scripts/check_docs.py
	python3 scripts/check_package.py
install:
	mkdir -p "$(DESTDIR)/bin" "$(DESTDIR)/lib/elxtools"
	install -m 755 build/elxstat build/elxbat build/elxctl build/elxdash "$(DESTDIR)/bin/"
	install -m 644 LICENSE "$(DESTDIR)/lib/elxtools/copying"
	install -m 644 docs/FONT-LICENSE "$(DESTDIR)/lib/elxtools/font.lic"
	install -m 644 USAGE.TXT "$(DESTDIR)/lib/elxtools/usage.txt"
