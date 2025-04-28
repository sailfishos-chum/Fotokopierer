target = harbour-fotokopierer

arch := i486
#arch := armv7hl
sfos_version := 3.2.1.20
device := jolla

sdk_dir := $(HOME)/SailfishOS
projects_root := $(HOME)/JollaProjekte
emu_dir := $(sdk_dir)/vmshare/ssh/private_keys/Sailfish_OS-Emulator-latest

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(mkfile_path))
mer_root_dir := $(subst $(HOME),/home/src1,$(current_dir))

mersdk_target := SailfishOS-$(sfos_version)-$(arch)
mersdk_device := Sailfish OS Emulator $(sfos_version)
mersdk_ssh := ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost
mersdk_mb2 := cd $(mer_root_dir) && mb2 -t $(mersdk_target)

ifeq ($(arch),i486)
  mersdk_sb2 := cd $(mer_root_dir)/rpmbuilddir-i386 && sb2 -t $(mersdk_target)
else
  mersdk_sb2 := cd $(mer_root_dir)/rpmbuilddir-arm && sb2 -t $(mersdk_target)
endif

emu_ssh := ssh -p 2223 -i $(emu_dir)/nemo nemo@localhost
emu_ssh_root := ssh -p 2223 -i $(emu_dir)/root root@localhost

TRANSLATIONS = de

.PHONY: all build buildall clean install rpm run deploy-emu
all: compile

reformat:
	clang-format -i --style=file src/*xx

installdeps:
	$(mersdk_ssh) '$(mersdk_mb2) build-requires'

build: reformat lrelease
	$(mersdk_ssh) '$(mersdk_mb2) build'

compile: reformat lrelease
	$(mersdk_ssh) '$(mersdk_sb2) make'

make:
	$(mersdk_ssh) '$(mersdk_mb2) make'

install:
	$(mersdk_ssh) '$(mersdk_mb2) make-install'

rpm: lrelease
	touch rpm/*.yaml
	$(mersdk_ssh) '$(mersdk_mb2) package'

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
