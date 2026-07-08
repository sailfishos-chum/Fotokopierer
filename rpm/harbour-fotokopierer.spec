Name:       harbour-fotokopierer

%global opencv_version 3.4.20
%global podofo_version 0.10.6
%global freetype_version 2.13.3

Summary:    Document Scanner
Version:    1.0.1
Release:    1%{?dist}
Group:      Qt/Qt
License:    GPLv3+
URL:        https://codeberg.org/fifr/Fotokopierer
Source0:    %{name}-%{version}.tar.gz
#Source1:    opencv-%{opencv_version}.tar.gz
Source2:    podofo-%{podofo_version}.tar.gz
#Source3:    freetype-%{freetype_version}.tar.gz
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(openssl) >= 1.1
BuildRequires:  pkgconfig(libpng16)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  cmake
BuildRequires:  ninja
BuildRequires:  libxml2-devel
BuildRequires:  qt5-qttools-linguist
BuildRequires:  qt5-qtmultimedia-plugin-audio-alsa
BuildRequires:  qt5-qtmultimedia-plugin-audio-pulseaudio
BuildRequires:  qt5-qtmultimedia-plugin-mediaservice-gstaudiodecoder
BuildRequires:  qt5-qtmultimedia-plugin-mediaservice-gstcamerabin
BuildRequires:  qt5-qtmultimedia-plugin-mediaservice-gstmediacapture
BuildRequires:  qt5-qtmultimedia-plugin-mediaservice-gstmediaplayer
BuildRequires:  qt5-qtmultimedia-plugin-mediaservice-irisradio
BuildRequires:  qt5-qtmultimedia-plugin-mediaservice-irisradio
BuildRequires:  qt5-qtmultimedia-plugin-playlistformats-m3u
BuildRequires:  qt5-qtmultimedia-plugin-resourcepolicy-resourceqt
BuildRequires:  qt5-qtmultimedia-plugin-video-eglvideonode
BuildRequires:  desktop-file-utils

Buildrequires: pkgconfig(freetype2) >= %{freetype_version}
#Buildrequires: pkgconfig(opencv) >= %%{opencv_version}
Buildrequires: pkgconfig(opencv4)
Buildrequires: pkgconfig(libjpeg)
Buildrequires: pkgconfig(libtiff-4)

%description
A camera-scanning application for Sailfish OS.

%if 0%{?_chum}
Title: Fotokopierer
Type: desktop-application
DeveloperName: Frank Fischer
Categories:
  - Graphics
  - Office
Custom:
  Repo: https://codeberg.org/fifr/Fotokopierer
  Icon: https://codeberg.org/fifr/Fotokopierer/raw/branch/main/icons/harbour-fotokopierer.svg
Screenshots:
  - https://codeberg.org/fifr/Fotokopierer/raw/branch/main/images/screenshot1.png
  - https://codeberg.org/fifr/Fotokopierer/raw/branch/main/images/screenshot2.png
  - https://codeberg.org/fifr/Fotokopierer/raw/branch/main/images/screenshot3.png
  - https://codeberg.org/fifr/Fotokopierer/raw/branch/main/images/screenshot4.png
PackageIcon: https://codeberg.org/fifr/Fotokopierer/raw/branch/main/icons/harbour-fotokopierer.svg
Links:
  Homepage: https://codeberg.org/fifr/Fotokopierer
  Bugtracker: https://codeberg.org/fifr/Fotokopierer/issues
%endif

%define __requires_exclude ^libcrypto|libjpeg|libopencv_core|libopencv_imgproc|libtiff|libfreetype.*$

%prep
%setup -q -n %{name}-%{version}

mkdir -p 3rdparty
pushd 3rdparty
#test -d opencv-%%{opencv_version} || tar -xzf %%{SOURCE1}
test -d podofo-%{podofo_version} || tar -xzf %{SOURCE2}
#test -d freetype-%%{freetype_version} || tar -xzf %%{SOURCE3}
popd

%build

%cmake \
    -DFREETYPE_VERSION=$(pkg-config --modversion freetype2) \
    -DOPENCV_VERSION=$(pkg-config --modversion opencv4) \
    %{nil}

%cmake_build

%install
%cmake_install

desktop-file-install --delete-original       \
  --dir %{buildroot}%{_datadir}/applications             \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}
%{_datadir}/%{name}/icons/*.svg
%{_datadir}/%{name}/qml/common
%{_datadir}/%{name}/qml/sfos
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
