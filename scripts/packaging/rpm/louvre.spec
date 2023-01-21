Name:           %{getenv:LOUVRE_RPM_PACKAGE_NAME}
Version:        %{getenv:LOUVRE_VERSION}
Release:        %{getenv:LOUVRE_BUILD}
Summary:        C++ library for Wayland compositors development.
BuildArch:      x86_64

License:        GPL-3.0
Source0:        %{name}-%{version}.tar.gz

Requires:       make automake gcc gcc-c++ kernel-devel libwayland-server wayland-devel libinput-devel libevdev-devel libudev-devel mesa-libEGL-devel libxkbcommon-devel libXcursor-devel pixman-devel libdrm-devel libgbm-devel libXrandr-devel libseat-devel


%description
This package contains the development library, backends, headers and examples of Louvre, a C++ library for developing Wayland compositors.

%prep
%setup -q

%install
cp -r ./ $RPM_BUILD_ROOT/

%files
