%define checkout_tag 2012052812snap

Name:				davix
Version:			0.0.2
Release:			0.1.%{checkout_tag}%{?dist}
Summary:			High level library for HTTP/WebDav file operations
Group:				Applications/Internet
License:			ASL 2.0
URL:				https://svnweb.cern.ch/trac/lcgdm/wiki
# svn export http://svn.cern.ch/guest/lcgdm/davix/trunk davix
Source0:			http://grid-deployment.web.cern.ch/grid-deployment/dms/lcgutil/tar/%{name}/%{name}-%{version}-%{checkout_tag}.tar.gz 
BuildRoot:			%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

#main lib dependencies
BuildRequires:		cmake
BuildRequires:		doxygen
BuildRequires:		glib2-devel
BuildRequires:		glibmm24-devel
BuildRequires:		neon-devel
BuildRequires:		libxml++-devel

%description
Davix provides an High level API for file operations in Grid and Cloud environments.

%package devel
Summary:			Development files for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 
Requires:			glib2-devel%{?_isa}
Requires:			glibmm24-devel%{?_isa}
Requires:			neon-devel%{?_isa}
Requires:			pkgconfig

%description devel
development files for %{name}

%package doc
Summary:			Documentation for %{name}
Group:				Applications/Internet
Requires:			%{name}-core%{?_isa} = %{version}-%{release} 

%description doc
documentation, Doxygen and examples of %{name} .

%clean
rm -rf %{buildroot};
make clean

%prep
%setup -q

%build
%cmake -DDOC_INSTALL_DIR=%{_docdir}/%{name}-%{version} .
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr (-,root,root)
%{_libdir}/libdavix.so.*
%{_docdir}/%{name}-%{version}/RELEASE-NOTES

%files devel
%defattr (-,root,root)
%{_libdir}/libdavix.so
%{_includedir}/davix/*
%{_libdir}/pkgconfig/*

%files doc
%defattr (-,root,root)
%{_docdir}/%{name}-%{version}/html/*

%changelog
* Fri Jun 01 2012 Adrien Devresse <adevress at cern.ch> - 0.0.2-0.1-2012052812snap
 - initial preview
