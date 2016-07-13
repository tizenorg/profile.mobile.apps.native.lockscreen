Name:       org.tizen.lockscreen
Summary:    Lockscreen app
Group:      Applications/Core Applications
Version:    0.0.1
Release:    1
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{profile}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{profile}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-tools
BuildRequires:  hash-signer
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(capi-system-runtime-info)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-ui-efl-util)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(minicontrol-viewer)
BuildRequires:  pkgconfig(feedback)
BuildRequires:  pkgconfig(tzsh-lockscreen-service)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(capi-base-utils-i18n)
BuildRequires:  pkgconfig(capi-telephony)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(auth-fw)
BuildRequires:  pkgconfig(dpm)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(capi-ui-efl-util)
BuildRequires:  pkgconfig(capi-message-port)
BuildRequires:  pkgconfig(capi-appfw-package-manager)

%description
Lockscreen application for Tizen.

%prep
%setup -q

%build

%define _pkg_dir %{TZ_SYS_RO_APP}/%{name}
%define _pkg_shared_dir %{_pkg_dir}/shared
%define _pkg_data_dir %{_pkg_dir}/data
%define _sys_icons_dir %{_pkg_shared_dir}/res
%define _sys_packages_dir %{TZ_SYS_RO_PACKAGES}
%define _sys_license_dir %{TZ_SYS_SHARE}/license

%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif

%ifarch %{arm}
export CFLAGS="$CFLAGS -DTIZEN_BUILD_TARGET"
export CXXFLAGS="$CXXFLAGS -DTIZEN_BUILD_TARGET"
export FFLAGS="$FFLAGS -DTIZEN_BUILD_TARGET"
%else
export CFLAGS="$CFLAGS -DTIZEN_BUILD_EMULATOR"
export CXXFLAGS="$CXXFLAGS -DTIZEN_BUILD_EMULATOR"
export FFLAGS="$FFLAGS -DTIZEN_BUILD_EMULATOR"
%endif


cd CMake
cmake . -DINSTALL_PREFIX=%{_pkg_dir} \
	-DSYS_ICONS_DIR=%{_sys_icons_dir} \
	-DSYS_PACKAGES_DIR=%{_sys_packages_dir}
make %{?jobs:-j%jobs}
cd -

%install
cd CMake
%make_install
cd -

%define tizen_sign 1
%define tizen_sign_base %{_pkg_dir}
%define tizen_sign_level public
%define tizen_author_sign 1
%define tizen_dist_sign 1
%find_lang lockscreen

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files -f lockscreen.lang
%manifest org.tizen.lockscreen.manifest
%defattr(-,root,root,-)
%{_pkg_dir}/bin/lockscreen
%{_pkg_dir}/res/images/*.jpg
%{_pkg_dir}/res/images/*.png
%{_pkg_dir}/res/edje/*.edj
%{_pkg_dir}/res/locale/*/LC_MESSAGES/*
%{_sys_packages_dir}/org.tizen.lockscreen.xml
%{_sys_icons_dir}/lockscreen.png
%{_pkg_dir}/author-signature.xml
%{_pkg_dir}/signature1.xml
