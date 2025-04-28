target = harbour-fotokopierer

arch := i486
#arch := armv7hl
#arch := aarch64

device := jolla

sdk_dir := $(HOME)/SailfishOS
sfdk := $(sdk_dir)/bin/sfdk

projects_root := $(HOME)/JollaProjekte
emu_dir := $(sdk_dir)/vmshare/ssh/private_keys/Sailfish_OS-Emulator-latest

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(mkfile_path))
mer_root_dir := $(current_dir)

mersdk_ssh := ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost

ifeq ($(arch),i486)
  build_dir := rpmbuilddir-i386
else
ifeq ($(arch),armv7hl)
  build_dir := rpmbuilddir-arm
else
  build_dir := rpmbuilddir-aarch64
endif
endif

emu_ssh := ssh -p 2223 -i $(emu_dir)/nemo nemo@localhost
emu_ssh_root := ssh -p 2223 -i $(emu_dir)/root root@localhost

TRANSLATIONS = de sv

.PHONY: all build buildall clean install rpm run deploy-emu
all: compile

reformat:
	clang-format -i --style=file src/*xx

installdeps:
	sf

build: reformat lrelease
	$(sfdk) build

compile: reformat lrelease
	$(sfdk) build-shell make -C $(build_dir) -j4

.PHONY: make
make: compile

install:
	$(sfdk) make-install

rpm: lrelease
	touch rpm/*.yaml
	$(sfdk) package

deploy-emu: all rpm
	scp -P 2223 -i $(emu_dir)/nemo RPMS/* nemo@localhost:
	$(emu_ssh_root) 'rpm --reinstall /home/nemo/$(target)-*.i486.rpm'

.PHONY: install-jolla copy-jolla run-jolla
rpm-jolla: rpm
	scp RPMS/harbour-fotokopierer*.armv7hl.rpm $(device):

install-jolla:
	scp rpmbuilddir-arm/harbour-fotokopierer $(device):

run-jolla:
	ssh -tt $(device) './harbour-fotokopierer'

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
