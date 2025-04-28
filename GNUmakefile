target = harbour-fotokopierer

arch := i486
#arch := armv7hl

sdk_dir := $(HOME)/SailfishOS
sfos_version := SailfishOS-3.0.2.8-$(arch)
projects_root := $(HOME)/JollaProjekte

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(mkfile_path))
mer_root_dir := $(subst $(HOME)/JollaProjekte,/home/src1,$(current_dir))


.PHONY: all build buildall clean install rpm run
all: build

buildall:
	ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost \
	'cd $(mer_root_dir) && mb2 -t $(sfos_version) build'

build:
	ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost \
	'cd $(mer_root_dir)/rpmbuilddir && mb2 -t $(sfos_version) make'

clean:
	ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost \
	'cd $(mer_root_dir)/rpmbuilddir && mb2 -t $(sfos_version) make clean'

rpm:
	ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost \
	'cd $(mer_root_dir) && mb2 -t $(sfos_version) rpm'

install:
	#ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost \
	#'cd $(mer_root_dir) && mb2 -t $(sfos_version) rpm'
	ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost \
	'cd $(mer_root_dir) && mb2 --device "Sailfish OS Emulator 3.0.2.8" deploy --sdk'

installdeps:
	#ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost \
	#'cd $(mer_root_dir) && mb2 -t $(sfos_version) rpm'
	ssh -p 2222 -i $(sdk_dir)/vmshare/ssh/private_keys/engine/mersdk mersdk@localhost \
	'cd $(mer_root_dir) && mb2 --device "Sailfish OS Emulator 3.0.2.8" installdeps'

run:
	ssh -tt -p 2223 -i $(sdk_dir)/vmshare/ssh/private_keys/Sailfish_OS-Emulator-latest/nemo nemo@localhost 'sh -c "env LD_LIBRARY_PATH=/usr/local/lib $(target) ${ARGS}"'

.PHONY: install-jolla copy-jolla run-jolla
install-jolla: rpm
	scp RPMS/harbour-fotokopierer*.armv7hl.rpm jolla:

copy-jolla:
	scp rpmbuilddir/harbour-fotokopierer jolla:

run-jolla: copy-jolla
	ssh -tt jolla './harbour-fotokopierer'
