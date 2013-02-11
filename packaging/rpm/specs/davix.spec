Name:				davix
Version:			0.0.24
Release:			0.1%{?dist}
Summary:			Webdav file access and file management Library
Group:				Applications/Internet
License:			LGPLv2+
URL:				https://svnweb.cern.ch/trac/lcgdm/wiki
# svn export http://svn.cern.ch/guest/lcgdm/davix/trunk davix
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}.tar.gz
BuildRoot:			%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

#main lib dependencies
BuildRequires:		cmake
BuildRequires:		doxygen
BuildRequires:		glib2-devel
BuildRequires:		libxml2-devel
BuildRequires:		openssl-devel

%description
Davix is a toolkit for remote file operations
with Webdav and Http based protocols.
Davix provides an API and a set of command line tools.

%package libs
Summary:			Development files for %{name}
Group:				Applications/Internet

%description libs
Libraries for %{name}. Davix is a toolkit for remote
file operations with Webdav and Http based protocols.


%package devel
Summary:			Development files for %{name}
Group:				Applications/Internet
Requires:			%{name}-libs%{?_isa} = %{version}-%{release}
Requires:			glib2-devel%{?_isa}
Requires:			pkgconfig

%description devel
Development files for %{name}. Davix is a toolkit for remote
file operations with Webdav and Http based protocols.

%package doc
Summary:			Documentation for %{name}
Group:				Applications/Internet
Requires:			%{name}%{?_isa} = %{version}-%{release}

%description doc
Documentation and examples for %{name}. Davix is a toolkit for remote
file operations with Webdav and Http based protocols.

%clean
rm -rf %{buildroot};
make clean

%prep
%setup -q

%build
%cmake \
-DDOC_INSTALL_DIR=%{_docdir}/%{name}-%{version} \
-D UNIT_TESTS=TRUE .
make
make doc

%check
ctest -V


%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%post libs -p /sbin/ldconfig

%postun libs -p /sbin/ldconfig

%files
%{_bindir}/*

%files libs
%defattr (-,root,root)
%{_libdir}/libdavix.so.*
%{_docdir}/%{name}-%{version}/RELEASE-NOTES

%files devel
%defattr (-,root,root)
%{_libdir}/libdavix.so
%dir %{_includedir}/davix
%{_includedir}/davix/*
%{_libdir}/pkgconfig/*

%files doc
%defattr (-,root,root)
%{_docdir}/%{name}-%{version}/html/*

%changelog
* Wed Feb 06 2013 Adrien Devresse <adevress at cern.ch> - 0.0.24-0.1
 - add cmd line tools
 - lot of dev work

* Fri Jun 01 2012 Adrien Devresse <adevress at cern.ch> - 0.0.2-0.1-2012052812snap
 - initial preview
