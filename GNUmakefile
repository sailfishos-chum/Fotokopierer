program = harbour-fotokopierer

sdk_dir := $(HOME)/SailfishOS
sfdk := $(sdk_dir)/bin/sfdk

arch := i486
#arch := armv7hl
arch := i486
#arch := aarch64

# Select the latest available target for the given architecture
#target := $(shell $(sfdk) tools list | awk -F' ' '/$(arch)/ { print $$2 }' | tail -n1)
target := SailfishOS-3.4.0.24-$(arch)

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

TRANSLATIONS = de sv sk

.PHONY: all
all: compile

.PHONY: reformat
reformat:
	clang-format -i --style=file src/*xx

.PHONY: build
build: reformat lrelease
	$(sfdk) -c "target=$(target)" build

.PHONY: compile
compile: reformat lrelease
	$(sfdk) -c "target=$(target)" build-shell make -C $(build_dir) -j4

.PHONY: make
make: compile

$(build_dir)/$(program): make

.PHONY: install
install:
	$(sfdk) -c "target=$(target)" make-install

rpm_version := $(shell cat rpm/$(program).yaml | awk '/^Version:/ {print $$2}')
rpm_release := $(shell cat rpm/$(program).yaml | awk '/^Release:/ {print $$2}' | awk -F% '{print $$1}')
rpm_file := RPMS/$(program)-$(rpm_version)-$(rpm_release).$(arch).rpm

.PHONY: rpm
rpm:
	$(MAKE) lrelease $(rpm_file)

$(rpm_file): $(build_dir)/$(program) rpm/$(program).yaml rpm/$(program).changes
	touch rpm/*.yaml
	$(sfdk) -c "target=$(target)" package

.PHONY: deploy-emu
deploy-emu: $(rpm_file)
	$(sfdk) -c "device=$(emulator)" deploy --rsync

.PHONY: run-emu
run-emu:
	$(sfdk) emulator exec /opt/sdk/$(program)/usr/bin/$(program)

# Translations
$(TRANSLATIONS:%=translations/harbour-fotokopierer-%.qm): %.qm: %.po
	lrelease $<

.PHONY: lupdate lrelease
lupdate:
	lupdate -locations relative src qml -ts translations/harbour-fotokopierer.pot $(TRANSLATIONS:%=translations/harbour-fotokopierer-%.po)
	sed -i -e "s!^#: ${current_dir}!#: !" translations/harbour-fotokopierer.pot $(TRANSLATIONS:%=translations/harbour-fotokopierer-%.po)

lrelease: translations.qrc

translations.qrc: $(TRANSLATIONS:%=translations/harbour-fotokopierer-%.qm)
	@echo "<RCC>" > $@
	@echo "  <qresource>" >> $@
	@printf "    <file>%s</file>\n" $(TRANSLATIONS:%="translations/harbour-fotokopierer-%.qm") >> $@
	@echo "  </qresource>" >> $@
	@echo "</RCC>" >> $@

snapshot_version := $(shell fossil info | awk '/^checkout:/ {print substr($$2, 1, 8)}')
snapshot:
	sed -i -e 's/^Release: 1%{?dist}.*$$/Release: 1%{?dist}.fossil+${snapshot_version}/' rpm/harbour-fotokopierer.yaml
	sed -i -e 's/FOTOKOPIERER_VERSION="$${FOTOKOPIERER_VERSION.*"/FOTOKOPIERER_VERSION="$${FOTOKOPIERER_VERSION}.fossil+${snapshot_version}"/' CMakeLists.txt

clean:
	$(sfdk) build-shell make -C $(build_dir) clean
	rm -rf 3rdparty/*-$(arch)
