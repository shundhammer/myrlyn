#
# spec file for package myrlyn
#
# Copyright (c) 2025 SUSE LLC
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#


%define         libzypp_devel_version libzypp-devel >= 17.21.0
Name:           myrlyn
# If you change the version here, don't forget ../VERSION.cmake !
# To increase only the last number, use  rake version:bump
Version:        0.9.8
Release:        0
Summary:        Package manager GUI
License:        GPL-2.0-only
Group:          System/Packages
URL:            https://github.com/shundhammer/myrlyn
Source:         %{name}-%{version}.tar.zst
BuildRequires:  %{libzypp_devel_version}
BuildRequires:  ImageMagick
BuildRequires:  boost-devel
BuildRequires:  cmake >= 3.17
BuildRequires:  hicolor-icon-theme
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(Qt6Core) >= 6.5
BuildRequires:  pkgconfig(Qt6Gui)
BuildRequires:  pkgconfig(Qt6Widgets)
# Not needed:   pkgconfig(Qt6Svg)
# We only need the image format plugin for SVG:
Requires:       libQt6Svg6

%if 0%{?suse_version} < 1600
BuildRequires:  gcc13
BuildRequires:  gcc13-c++
%else
BuildRequires:  gcc
BuildRequires:  gcc-c++
%endif
# Only needed for using the 'myrlyn-xdg-su.desktop' file
Recommends:	xdg-utils
# Obsolete accidential release because of a wrong tag on GitHub (9.0.1 instead of 0.9.1)
Obsoletes:      myrlyn = 9.0.1

%description
Myrlyn is a graphical package manager to select software packages
and patterns for installation, update and removal.

It uses libzypp as its backend and Qt as its GUI toolkit.

%prep
%autosetup -p1

%build
%if 0%{?suse_version} < 1600
export CC=gcc-13
export CXX=g++-13
%endif
export CFLAGS="%{optflags} -fPIE"
export CXXFLAGS="%{optflags} -fPIE"
export LDFLAGS="-pie"

%if %{?_with_debug:1}%{!?_with_debug:0}
CMAKE_OPTS="-DCMAKE_BUILD_TYPE=RELWITHDEBINFO"
%else
CMAKE_OPTS="-DCMAKE_BUILD_TYPE=RELEASE"
%endif

%cmake \
  $CMAKE_OPTS \
  -DUSE_RPM_GROUPS=off
%cmake_build

%install
%cmake_install


%files
%doc README.md
%license LICENSE
%{_bindir}/myrlyn
%{_bindir}/myrlyn-askpass
%{_bindir}/myrlyn-sudo
%{_datadir}/applications/%{name}-*.desktop
%{_datadir}/icons/hicolor/*/apps/Myrlyn.png

%changelog
