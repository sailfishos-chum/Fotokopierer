target = harbour-fotokopierer

arch := i486
#arch := armv7hl
sfos_version := 3.0.3.9
device := jolla

sdk_dir := $(HOME)/SailfishOS
projects_root := $(HOME)/JollaProjekte

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(mkfile_path))
mer_root_dir := $(subst $(HOME)/JollaProjekte,/home/src1,$(current_dir))

mersdk_target := SailfishOS-$(sfos_version)-$(arch)
mersdk_device := Sailfish OS Emulator $(sfos_version)
mersdk_ssh := ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost
mersdk_mb2 := cd $(mer_root_dir) && mb2 -t $(mersdk_target)
mersdk_sb2 := cd $(mer_root_dir)/rpmbuilddir-arm && sb2 -t $(mersdk_target)

TRANSLATIONS = de

.PHONY: all build buildall clean install rpm run
all: compile

reformat:
	clang-format -i --style=file src/*xx

installdeps:
	$(mersdk_ssh) '$(mersdk_mb2) installdeps'

build: reformat
	$(mersdk_ssh) '$(mersdk_mb2) build'

compile: reformat
	$(mersdk_ssh) '$(mersdk_sb2) make'

make:
	$(mersdk_ssh) '$(mersdk_mb2) make'

install:
	$(mersdk_ssh) '$(mersdk_mb2) install'

rpm:
	$(mersdk_ssh) '$(mersdk_mb2) rpm'

deploy:
	$(mersdk_ssh) '$(mersdk_mb2) --device "$(mersdk_device)" deploy --pkcon'

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
