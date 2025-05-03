PROJECT := "fotokopierer"
BIN := "/usr/bin/harbour-fotokopierer"

TARGET := "SailfishOS-5.0.0.62"
DEVICE := "Sailfish OS Emulator 5.0.0.62"
SDKDIR := "~/SailfishOS"

sfdk := SDKDIR / "bin" / "sfdk"

config arch:
    {{sfdk}} config target="{{TARGET}}-{{arch}}"
    {{sfdk}} config device="{{DEVICE}}"
    {{sfdk}} config --push no-fix-version

build mode="debug" arch="i486": (cmake mode arch) (make mode arch)

cmake mode="debug" arch="i486": (config arch)
    mkdir -p ../build-{{PROJECT}}-{{arch}}-{{mode}}
    cd ../build-{{PROJECT}}-{{arch}}-{{mode}} && {{sfdk}} cmake {{justfile_directory()}} 

make mode="debug" arch="i486": (config arch)
    cd ../build-{{PROJECT}}-{{arch}}-{{mode}} && {{sfdk}} make -j2

clean mode="debug" arch="i486": (config arch)
    cd ../build-{{PROJECT}}-{{arch}}-{{mode}} && {{sfdk}} make -j2 clean

package mode="debug" arch="i486": (make mode arch)
    mkdir -p ../build-{{PROJECT}}-{{arch}}-{{mode}}
    cd ../build-{{PROJECT}}-{{arch}}-{{mode}} && {{sfdk}} package

emulator mode="debug" arch="i486": (package mode arch)
    cd ../build-{{PROJECT}}-{{arch}}-{{mode}} && {{sfdk}} deploy --sdk
    ssh -p 2223 -i "{{SDKDIR}}/vmshare/ssh/private_keys/sdk" defaultuser@localhost {{BIN}}

shutdown:
    {{sfdk}} emulator stop
    {{sfdk}} engine stop

allrpms: (package "release" "i486") (package "release" "aarch64") (package "release" "armv7hl")
