target = harbour-fotokopierer

arch := i486
#arch := armv7hl
sfos_version := 3.0.2.8

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


.PHONY: all build buildall clean install rpm run
all: build

installdeps:
	$(mersdk_ssh) '$(mersdk_mb2) installdeps'

build:
	$(mersdk_ssh) '$(mersdk_mb2) build'

compile:
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
	scp RPMS/harbour-fotokopierer*.armv7hl.rpm jolla:

install-jolla: make
	scp rpmbuilddir-arm/harbour-fotokopierer jolla:

run-jolla:
	ssh -tt jolla './harbour-fotokopierer'
