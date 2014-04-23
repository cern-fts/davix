# unversionned doc dir F20 change https://fedoraproject.org/wiki/Changes/UnversionedDocdirs
%{!?_pkgdocdir: %global _pkgdocdir %{_docdir}/%{name}-%{version}}

# EL5 compat for boost, cmake
%if 0%{?el5}
%global boost_cmake_flags -DBOOST_INCLUDEDIR=/usr/include/boost141 -DBOOST_LIBRARYDIR=%{_libdir}/boost141
%else
%global boost_cmake_flags -DBOOST_INCLUDEDIR=/usr/include
%endif

Name:				davix
Version:			0.2.11
Release:			11%{?dist}
Summary:			Toolkit for Http-based file management
Group:				Applications/Internet
License:			LGPLv2+
URL:				http://dmc.web.cern.ch/projects/davix/home
# git clone http://git.cern.ch/pub/davix
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}.tar.gz
BuildRoot:			%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

#main lib dependencies
%if 0%{?el5}
BuildRequires:                  boost141-devel
%else
BuildRequires:                  boost-devel
%endif
BuildRequires:                  cmake
BuildRequires:                  doxygen
BuildRequires:                  libxml2-devel
BuildRequires:                  openssl-devel

# davix-copy dependencies
BuildRequires:                  gridsite-devel
BuildRequires:                  gsoap-devel
# unit tests
BuildRequires:                  gtest-devel


Requires:                       %{name}-libs%{?_isa} = %{version}-%{release}



%description
Davix is a toolkit designed for file operations
with Http based protocols (WebDav, Amazon S3, ...).
Davix provides an API and a set of command line tools.

%package libs
Summary:			Development files for %{name}
Group:				Applications/Internet

%description libs
Libraries for %{name}. Davix is a toolkit designed for file operations
with Http based protocols (WebDav, Amazon S3, ...).


%package devel
Summary:			Development files for %{name}
Group:				Applications/Internet
Requires:			%{name}-libs%{?_isa} = %{version}-%{release}
Requires:			pkgconfig

%description devel
Development files for %{name}. Davix is a toolkit designed for file operations
with Http based protocols (WebDav, Amazon S3, ...).

%package doc
Summary:			Documentation for %{name}
Group:				Documentation
%if %{?fedora}%{!?fedora:0} >= 10 || %{?rhel}%{!?rhel:0} >= 6
BuildArch:	noarch
%endif

%description doc
Documentation and examples for %{name}. Davix is a toolkit designed 
for file operations with Http based protocols (WebDav, Amazon S3, ...).

%clean
rm -rf %{buildroot}
make clean

%prep
%setup -q
# remove useless embedded components
rm -rf test/pywebdav/

%build
%cmake \
-DDOC_INSTALL_DIR=%{_pkgdocdir} \
-DGTEST_EXTERNAL=TRUE \
-DENABLE_THIRD_PARTY_COPY=TRUE \
-DUNIT_TESTS=TRUE \
%{boost_cmake_flags} \
.
make %{?_smp_mflags}
make doc

%check
ctest -V -T Test


%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%post libs -p /sbin/ldconfig

%postun libs -p /sbin/ldconfig

%files
%defattr (-,root,root)
%{_bindir}/*

%files libs
%defattr (-,root,root)
%{_libdir}/libdavix.so.*
%{_libdir}/libdavix_copy.so.*


%files devel
%defattr (-,root,root)
%{_libdir}/libdavix.so
%{_libdir}/libdavix_copy.so
%dir %{_includedir}/davix
%{_includedir}/davix/*
%{_libdir}/pkgconfig/*

%files doc
%defattr (-,root,root)
%{_pkgdocdir}/LICENSE
%{_pkgdocdir}/RELEASE-NOTES
%{_pkgdocdir}/html/


%changelog
* Tue Jan 28 2014 Adrien Devresse <adevress at cern.ch> - 0.2.10-1
 - davix 0.2.10 release, see RELEASE-NOTES for details

* Mon Oct 28 2013 Adrien Devresse <adevress at cern.ch> - 0.2.7-3
 - New update of davix, see RELEASE-NOTES for details


* Tue Sep 03 2013 Adrien Devresse <adevress at cern.ch> - 0.2.6-1
 - Release 0.2.6 of davix, see RELEASE-NOTES for details


* Wed Jun 05 2013 Adrien Devresse <adevress at cern.ch> - 0.2.2-2
 - Initial EPEL release
 
 
