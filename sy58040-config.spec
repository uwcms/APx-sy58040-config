# Build this using apx-rpmbuild.
%define name sy58040-config

Name:           %{name}
Version:        %{version_rpm_spec_version}
Release:        %{version_rpm_spec_release}%{?dist}
Summary:        A GPIO-based utility to configure a SY58040 chip.

License:        Reserved
URL:            https://github.com/uwcms/APx-%{name}
Source0:        %{name}-%{version_rpm_spec_version}.tar.gz

%global debug_package %{nil}

%description
A GPIO-based utility to configure a SY58040 chip.


%prep
%setup -q


%build
##configure
make %{?_smp_mflags} LIB_VERSION=%{version_sofile}


%install
rm -rf $RPM_BUILD_ROOT
install -D -m 0755 sy58040-config %{buildroot}/%{_bindir}/sy58040-config


%files
%{_bindir}/sy58040-config

%changelog
* Tue Nov 26 2019 Jesra Tikalsky <jtikalsky@hep.wisc.edu>
- Initial spec file
