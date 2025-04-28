program = harbour-fotokopierer

sdk_dir := $(HOME)/SailfishOS
sfdk := $(sdk_dir)/bin/sfdk

arch := i486
#arch := armv7hl
arch := i486
#arch := aarch64

# Select the latest available target for the given architecture
target := $(shell $(sfdk) tools list | awk -F' ' '/$(arch)/ { print $$2 }' | tail -n1)

# Select the emulator device '#0'
emulator := $(shell $(sfdk) emulator list | awk -F'"' '/\#0/ { print $$2 }')

device := jolla

projects_root := $(HOME)/JollaProjekte
emu_dir := $(sdk_dir)/vmshare/ssh/private_keys/Sailfish_OS-Emulator-latest

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(mkfile_path))
mer_root_dir := $(current_dir)

ifeq ($(arch),i486)
  build_dir := rpmbuilddir-i386
else
ifeq ($(arch),armv7hl)
  build_dir := rpmbuilddir-arm
else
  build_dir := rpmbuilddir-$(arch)
endif
endif

TRANSLATIONS = de sv

.PHONY: all build buildall clean install rpm run deploy-emu
all: compile

reformat:
	clang-format -i --style=file src/*xx

installdeps:
	sf

build: reformat lrelease
	$(sfdk) -c "target=$(target)" build

compile: reformat lrelease
	$(sfdk) -c "target=$(target)" build-shell make -C $(build_dir) -j4

.PHONY: make
make: compile

install:
	$(sfdk) -c "target=$(target)" make-install

rpm: lrelease
	touch rpm/*.yaml
	$(sfdk) -c "target=$(target)" package

.PHONY: deploy-emu
deploy-emu:
	$(sfdk) -c "device=$(emulator)" deploy --rsync

.PHONY: run-emu
run-emu:
	$(sfdk) emulator exec /opt/sdk/$(program)/usr/bin/$(program)

# Translations
$(TRANSLATIONS:%=translations/harbour-fotokopierer-%.qm): %.qm: %.po
	lrelease $<

.PHONY: lupdate lrelease
lupdate:
	lupdate src qml -ts translations/harbour-fotokopierer.pot
	lupdate src qml -ts $(TRANSLATIONS:%=translations/harbour-fotokopierer-%.po)

lrelease: translations.qrc

translations.qrc: $(TRANSLATIONS:%=translations/harbour-fotokopierer-%.qm)
	@echo "<RCC>" > $@
	@echo "  <qresource>" >> $@
	@printf "    <file>%s</file>\n" $(TRANSLATIONS:%="translations/harbour-fotokopierer-%.qm") >> $@
	@echo "  </qresource>" >> $@
	@echo "</RCC>" >> $@

snapshot_version := $(shell fossil info | awk '/^checkout:/ {print "1%{?dist}.fossil+" substr($$2, 1, 8)}')
snapshot:
	sed -ie 's/^Release: 1%{?dist}.*$$/Release: ${snapshot_version}/' rpm/harbour-fotokopierer.yaml

clean:
	$(sfdk) build-shell make -C $(build_dir) clean
	rm -rf 3rdparty/*-$(arch)
